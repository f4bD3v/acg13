/*******************************************************************************
 *  path.cpp
 *******************************************************************************
 *  Copyright (c) 2013 Alexandre Kaspar <alexandre.kaspar@a3.epfl.ch>
 *  For Advanced Computer Graphics, at the LGG / EPFL
 *
 *        DO NOT REDISTRIBUTE
 ***********************************************/

#include <nori/acg.h>
#include <nori/integrator.h>
#include <nori/sampler.h>
#include <nori/luminaire.h>
#include <nori/bsdf.h>
#include <nori/medium.h>
#include <nori/phase.h>
#include <nori/scene.h>

NORI_NAMESPACE_BEGIN

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// put your group number here!
#define GROUP_NUMBER 10
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

GROUP_NAMESPACE_BEGIN()

/**
 * Simple path tracer implementation
 */
class PathTracer : public Integrator {
public:

        PathTracer(const PropertyList &) {
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
                        float G_lum = dp / dist2;

                        // 7. Radiance from luminaire
                        Color3f value = lRec.luminaire->getColor();

                        return value * G_lum * luminaires.size() / mesh->pdf();
                } else {
                        // wrong side of luminaire!
                        return Color3f(0.0f);
                }
        }

        Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &_ray) const {
                Ray3f ray(_ray);
                Intersection its;
                Color3f result(0.0f), throughput(1.0f);

                // TODO implement a path tracer

                // Step 1: Intersect the ray with the scene. Return environment
                // luminaire if no hit.
                    if (!scene->rayIntersect(ray, its))
                        return Color3f(0.0f);

                // Step 2: Check if the ray hits a light source.
                  const Mesh *mesh = its.mesh;
                  const BSDF *bsdf = mesh->getBSDF();

                  /* If we hit a luminaire, use its related color information */
                  if (mesh->isLuminaire()) {
                          const Luminaire *luminaire = its.mesh->getLuminaire();
                          LuminaireQueryRecord lRec(luminaire, ray.o, its.p, its.shFrame.n);
                          return luminaire->eval(lRec);
                  }

                // Step 3: Direct illumination sampling.
                // Step 4: Recursively sample indirect illumination

                int depthBound = 4;
                TVector<Point3f,4> positions;
                positions[0]=ray.o;
                positions[1]=its.p;


                for(int depth = 2; depth<depthBound; depth++){
                   // int i = std::max(2,depth-1);
                    throughput = 1.0f;
                    if(depth!=2){ // second-to-last point: sampling BRDF to find direction
                        Point3f x = positions[depth-3];
                        Point3f xprime = positions[depth-2];
                        BSDFQueryRecord bRec(its.toLocal(x-xprime));
                        bsdf->sample(bRec, sampler->next2D());
                        scene->rayIntersect(Ray3f(xprime,bRec.wo),its);
                        positions[depth-1]=its.p;
                        float dist2 = (its.p-xprime).squaredNorm();
                        if (scene->rayIntersect(Ray3f(its.p, (xprime-its.p), Epsilon , dist2 * (1 - 1e-4f)))){
                             throughput=Color3f(0.0f); // no visibility
                             //std::cout << "no visibility \n";
                        }
                        else {
                             float cos1 = Frame::cosTheta(bRec.wo);
                             float cos2 = Frame::cosTheta(its.toLocal(xprime-its.p));
                             //std::cout << "cos1 = " << cos1 << ", cos2 = " << cos2 << std::endl;
                             float G = cos1 * cos2 /dist2;
                             //std::cout << "G: " << G << std::endl;
                             Color3f f_r = bsdf->eval(bRec);
                             //std::cout << "BRDF = " << f_r << std::endl;
                             throughput = throughput * f_r * G;
                             //std::cout << "Throughput = " << throughput << std::endl;
                        }
                    }
                    // last point: looking for light source
                    Point3f x = positions[depth-2];
                    Point3f xprime = positions[depth-1];
                    LuminaireQueryRecord lRec(xprime);
                    Color3f Le = sampleLights(scene, lRec, sampler->next2D());
                    positions[depth]=lRec.p;
                    if ((Le.array() != 0).any()) {
                        BSDFQueryRecord bRec((xprime-x),
                        (lRec.p-xprime), ESolidAngle);
                        Color3f f_r = bsdf->eval(bRec);
                        //std::cout << "BSDF = " << f_r << std::endl;
                        result += (throughput * Le * f_r
                             * scene->evalTransmittance(Ray3f(lRec.ref, lRec.d, 0, lRec.dist), sampler)
                             * std::abs(Frame::cosTheta(bRec.wo)));
                    }
                }

                // Step 5. Apply Russion Roullette after 2 main bounces.
                //std::cout << "Result = " << result << std::endl;
                return result;
        }

        QString toString() const {
                return "PathTracer[]";
        }
};

GROUP_NAMESPACE_END

NORI_REGISTER_GROUP_CLASS(PathTracer, "path");
NORI_NAMESPACE_END
