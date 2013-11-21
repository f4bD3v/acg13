/*******************************************************************************
 *  light_integrator.cpp
 *******************************************************************************
 *  Copyright (c) 2013 Alexandre Kaspar <alexandre.kaspar@a3.epfl.ch>
 *  For Advanced Computer Graphics, at the LGG / EPFL
 *
 *        DO NOT REDISTRIBUTE
 ***********************************************/

#include <nori/acg.h>
#include <nori/bsdf.h>
#include <nori/common.h>
#include <nori/integrator.h>
#include <nori/luminaire.h>
#include <nori/mesh.h>
#include <nori/sampler.h>
#include <nori/scene.h>
#include <nori/vector.h>
#include <vector>

NORI_NAMESPACE_BEGIN

/**
 * \brief Simple local illumination integrator
 * using light area sampling
 */
class LightIntegrator : public Integrator {
public:

        LightIntegrator(const PropertyList &propList) {
                Q_UNUSED(propList);
        }

        /// Return the mesh corresponding to a given luminaire
        inline const Mesh *getMesh(const Luminaire *lum) const {
                const Mesh *mesh = dynamic_cast<const Mesh *> (lum->getParent());
                if (!mesh) throw NoriException("Unhandled type of luminaire!");
                return mesh;
        }

        /**
         * \brief Directly sample the lights, providing a sample weighted by 1/pdf
         * where pdf is the probability of sampling that given sample
         * 
         * \param scene
         * the scene to work with
         * 
         * \param lRec
         * the luminaire information storage
         * 
         * \param _sample
         * the 2d uniform sample
         * 
         * \return the sampled light radiance including its geometric, visibility and pdf weights
         */
        inline Color3f sampleLights(const Scene *scene, LuminaireQueryRecord &lRec, const Point2f &_sample) const {
                Point2f sample(_sample);
                const std::vector<Luminaire *> &luminaires = scene->getLuminaires();

                if (luminaires.size() == 0)
                        throw NoriException("LightIntegrator::sampleLights(): No luminaires were defined!");

                // 1. Choose one luminaire at random
                int index = std::min((int) (luminaires.size() * sample.x()), (int) luminaires.size() - 1);
                sample.x() = luminaires.size() * sample.x() - index; // process sample to be Unif[0;1] again

                // 2. Sample the position on the luminaire mesh
                // using Mesh::samplePosition(const Point2d &sample, Point3f &p, Normal3f &n)
                lRec.luminaire = luminaires[index];
                const Mesh *mesh = getMesh(lRec.luminaire);
                mesh->samplePosition(sample, lRec.p, lRec.n);
		lRec.d = lRec.p - lRec.ref;
		
                // 3. Compute distance between the two points (from first mesh, to luminaire mesh)
		float dist2 = lRec.d.squaredNorm();
		lRec.dist = std::sqrt(dist2);
		lRec.d /= lRec.dist;

                // 4. Correct side of luminaire
                // /!\ if on the wrong side, then we get no contribution!
		float dp = -lRec.n.dot(lRec.d);
		lRec.pdf = dp > 0 ? mesh->pdf() * dist2 / dp : 0.0f;
                
                if (dp > 0) {
                        // 5. Check the visibility
                        if (scene->rayIntersect(Ray3f(lRec.ref, lRec.d, Epsilon, lRec.dist * (1 - 1e-4f))))
                                return Color3f(0.0f);
                        // 6. Geometry term on luminaire's side
                        // Visiblity + Geometric term on the luminaire's side 
                        //      G(x, x', w, w') = ( cos(w) cos(w') ) / ||x - x'||^2
                        float G_lum = dp / dist2 / mesh->pdf();
                        
                        // 7. Radiance from luminaire
                        Color3f value = lRec.luminaire->getColor();
                        
                        return value * G_lum * luminaires.size();
                } else {
                        // wrong side of luminaire!
                        return Color3f(0.0f);
                }
        }

        /**
         * \brief Simplest (but clearly not smartest) local illumination integration:
         * We cast a ray from the camera, hit the first element in the scene it goes through
         * and uniformly sample a new ray to check whether we get any light.
         */
        Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray_) const {
                Ray3f ray(ray_);

                /* Find the surface that is visible in the requested direction */
                Intersection its;
                if (!scene->rayIntersect(ray, its))
                        return Color3f(0.0f);

                const Mesh *mesh = its.mesh;
                const BSDF *bsdf = mesh->getBSDF();

                /* If we hit a luminaire, use its related color information */
                if (mesh->isLuminaire()) {
                        const Luminaire *luminaire = its.mesh->getLuminaire();
                        LuminaireQueryRecord lRec(luminaire, ray.o, its.p, its.shFrame.n);
                        return luminaire->eval(lRec);
                }

                /* Sample a luminaire directly */
                LuminaireQueryRecord lRec(its.p);
                Color3f direct = sampleLights(scene, lRec, sampler->next2D());
                if ((direct.array() != 0).any()) {
                        BSDFQueryRecord bRec(its.toLocal(-ray.d),
                                its.toLocal(lRec.d), ESolidAngle);

                        return direct * bsdf->eval(bRec)
                                * scene->evalTransmittance(Ray3f(lRec.ref, lRec.d, 0, lRec.dist), sampler)
                                * std::abs(Frame::cosTheta(bRec.wo));
                }
                // else we don't get light
                return Color3f(0.0f);
        }

        QString toString() const {
                return QString("LightIntegrator[]");
        }
};

NORI_REGISTER_CLASS(LightIntegrator, "light");
NORI_NAMESPACE_END
