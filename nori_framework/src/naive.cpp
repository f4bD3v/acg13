/*******************************************************************************
 *  naive.cpp
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
 * using hemisphere sampling
 */
class NaiveIntegrator : public Integrator {
public:
	NaiveIntegrator(const PropertyList &propList) {
            Q_UNUSED(propList);
        }

	/**
        * \brief Uniform hemisphere sampling using the builtin sincosf fast math
        * routine to generate the cosine and sine of a value quickly.
        */
       static Vector3f hemisphereSampling(const Point2f &sample) {
               float cosTheta = sample.x();
               float sinTheta = std::sqrt(std::max((float) 0, 1-cosTheta*cosTheta));

               float sinPhi, cosPhi;
               sincosf(2.0f * M_PI * sample.y(), &sinPhi, &cosPhi);

               return Vector3f(cosPhi * sinTheta, sinPhi * sinTheta, cosTheta);
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
               const Vector3f wo = hemisphereSampling(sampler->next2D());
               BSDFQueryRecord bRec(its.toLocal(-ray.d), wo, ESolidAngle);
               const Color3f f_r = bsdf->eval(bRec);
               if((f_r.array() == 0).all()){
                       return Color3f(0);
               }
               const float inv_pdf = 2.0f * M_PI; // since hemisphere area is 2pi

               /* Find the surface that is visible in the sampled direction */
               ray = Ray3f(its.p, its.shFrame.toWorld(wo));
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
               // resulting color from light radiance, brdf, pdf and cosine of out angle
               return radiance * f_r * inv_pdf * std::abs(Frame::cosTheta(wo));
       }

        QString toString() const {
                return QString("NaiveIntegrator[]");
        }
};

NORI_REGISTER_CLASS(NaiveIntegrator, "naive");
NORI_NAMESPACE_END
