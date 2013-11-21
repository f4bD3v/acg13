/*******************************************************************************
 *  directional.cpp
 *******************************************************************************
 *  Copyright (c) 2013 Alexandre Kaspar <alexandre.kaspar@a3.epfl.ch>
 *  For Advanced Computer Graphics, at the LGG / EPFL
 * 
 *        DO NOT REDISTRIBUTE
 ***********************************************/

#include <nori/bsdf.h>
#include <nori/common.h>
#include <nori/integrator.h>
#include <nori/luminaire.h>
#include <nori/sampler.h>
#include <nori/scene.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Simple local illumination integrator
 * using directional sampling (from the BSDF)
 */
class DirectionalIntegrator : public Integrator {
public:
	DirectionalIntegrator(const PropertyList &propList) {
            Q_UNUSED(propList);
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

               /* Sample a direction on the hemisphere (naively) */
               BSDFQueryRecord bRec(its.toLocal(-ray.d));
               const Color3f throughput = bsdf->sample(bRec, sampler->next2D());
               if((throughput.array() == 0).all()){
                       return Color3f(0);
               }

               /* Find the surface that is visible in the sampled direction */
               ray = Ray3f(its.p, its.shFrame.toWorld(bRec.wo));
               bool hitSurface = scene->rayIntersect(ray, its);
               LuminaireQueryRecord lRec;

               // what do we hit?
               Color3f radiance(0.0f);
               if (!hitSurface) {
                       if (scene->hasEnvLuminaire()) {
                               /* A viewing ray hit the environment luminaire
                                  -> query the amount of emitted radiance */
                               lRec = LuminaireQueryRecord(
                                       scene->getEnvLuminaire(), ray);
                               radiance = lRec.luminaire->eval(lRec);
                       }
               } else if (its.mesh->isLuminaire()) {
                       lRec = LuminaireQueryRecord(
                               its.mesh->getLuminaire(),
                               ray.o, its.p, its.shFrame.n);
                       radiance = lRec.luminaire->eval(lRec);
               }
               // return the result color
               return throughput * radiance;
       }

        QString toString() const {
                return QString("DirectionalIntegrator[]");
        }
};

NORI_REGISTER_CLASS(DirectionalIntegrator, "directional");
NORI_NAMESPACE_END
