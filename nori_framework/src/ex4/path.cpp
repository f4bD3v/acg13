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

#define GROUP_NUMBER 10
#define probabilty_to_continue_eye 0.7
#define probabilty_to_continue_light 0.5

GROUP_NAMESPACE_BEGIN()

/**
 * Simple path tracer implementation
 */
class PathTracer:public Integrator {
public:

	PathTracer(const PropertyList &) {}

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
		int depth = 0;
		float eta = 1.0f;
		// whether to use the emitted light we would hit
		// - first hit => yes
		// - hit through a mirror => yes
		// - hit through something else => no (direct lighting already takes it into account)
		bool includeEmitted = true;

		// trace path from light
		std::vector<Intersection> itsL;
		std::vector<Color3f> throughputs;
////Intersection itsL[light_path_length];
////Color3f throughputs[light_path_length];
		// 1. Choose a random light
		const std::vector<Luminaire *> &luminaires = scene->getLuminaires();
		int index = std::min((int) (luminaires.size() * sampler->next1D()), (int) luminaires.size() - 1);
		const Luminaire *luminaire = luminaires[index];
		// 2. Choose a random point in the light
		const Mesh *mesh = getMesh(luminaire);
		Normal3f normal;
		Intersection it;
		itsL.push_back(it);
		mesh->samplePosition(sampler->next2D(), it.p, normal);
		// 3. Choose a random direction in the same half plane as the normal
		Vector3f direction = squareToCosineHemisphere(sampler->next2D());
		while (direction.dot(normal) <= 0)
			direction = squareToCosineHemisphere(sampler->next2D());
		// 4. Create the ray form the light in the random direction
		Ray3f rayL = Ray3f(it.p, direction);
		// 5. Compute initial throughput
		LuminaireQueryRecord lRec(it.p);
		throughputs.push_back(Color3f(0.0f));
		throughputs[0] = sampleLights(scene, lRec, sampler->next2D());
		throughputs[0] = scene->evalTransmittance(Ray3f(lRec.ref, lRec.d, 0, lRec.dist), sampler);
		// 6. Compute the light path
		unsigned int real_length = 1;
		while (true) {
			Color3f color;
			Intersection it;
			// test russian roulette
			if (sampler->next1D() >= probabilty_to_continue_light)
				break;
			// 6.a. Compute next intersection
			if (!scene->rayIntersect(rayL, it))
				break;
			// 6.b. Update throughput
			BSDFQueryRecord bRec(it.toLocal(-rayL.d));
			const BSDF *bsdf = it.mesh->getBSDF();
			Color3f bsdfWeight = bsdf->sample(bRec, sampler->next2D());
			if ((bsdfWeight.array() == 0).all())
				break;
			eta *= bRec.eta;
			// if the relative index got too small or too big, we stop
			if (eta > 2 || eta < 0.5) {
				cout << "OOps!" << endl;
				break;
			}
			color = throughputs[real_length-1] * bsdfWeight / probabilty_to_continue_light;
			// 6.c. Generate the new ray
			rayL = Ray3f(it.p, it.shFrame.toWorld(bRec.wo));
			// 6.d. Update structures
			itsL.push_back(it);
			throughputs.push_back(color);
			++real_length;
		}

		// trace path from eye
		while (true) {
			// 1. Intersect our ray with something
			scene->rayIntersect(ray, its);

			// 1.b. if we hit nothing, hit the environment
			//      luminaire if there is one, and stop the path
			if (its.t == std::numeric_limits<float>::infinity()) {
				if (includeEmitted && scene->hasEnvLuminaire()) {
					/* Hit an environment luminaire
					 -> query the amount of emitted radiance */
					const Luminaire *env = scene->getEnvLuminaire();
					LuminaireQueryRecord lRec(env, ray);
					result += throughput * env->eval(lRec);
				}
				break;
			}

			const BSDF *bsdf = its.mesh->getBSDF();

			// 2. Check whether the hit object emits light
			if (includeEmitted && its.mesh->isLuminaire()) {
				// L[DS]*DE paths are not accepted as they produce too much variance!
				// but L[DS]*SE path are since their direct lighting will have zero throughput
				//      (= discrete BSDF)
				// => add light contribution of the path
				const Luminaire *luminaire = its.mesh->getLuminaire();
				LuminaireQueryRecord lRec(luminaire, ray.o, its.p, its.shFrame.n);
				result += throughput * luminaire->eval(lRec);

				// Note: we may bounce out of a light, so we don't stop here
				//       but in practice, our scene won't use further paths
			}

			// 3. Direct illumination sampling
			LuminaireQueryRecord lRec(its.p);
			Color3f direct = sampleLights(scene, lRec, sampler->next2D());
			if ((direct.array() != 0).any()) {
				BSDFQueryRecord bRec(its.toLocal(-ray.d),
									 its.toLocal(lRec.d), ESolidAngle);
				// Note: evalTransmittance is 1.0f in our scenes, so we could just skip it
				result += throughput * direct * bsdf->eval(bRec)
				* scene->evalTransmittance(Ray3f(lRec.ref, lRec.d, 0, lRec.dist), sampler)
				* std::abs(Frame::cosTheta(bRec.wo)) ;
			}

			// 4. Combine eye and light paths
			// try to combine the current its to each point of the light path
			// except the first point which is on the light
			for (unsigned int i = 1; i < real_length; ++i) {
				// already in the throughputs
				// - direct
				// - transmittance
				BSDFQueryRecord bRec(its.toLocal(-ray.d),
									 its.toLocal(itsL[i].p - its.p).normalized(), ESolidAngle);
				// test if obstacle in the way
				Ray3f clear = Ray3f(itsL[i].p, its.p - itsL[i].p);
				Intersection its_tmp;
				scene->rayIntersect(clear, its_tmp);
				if ((its_tmp.p - itsL[i].p).squaredNorm() > (its.p - itsL[i].p).squaredNorm())
					// update result by combining the throughputs and the bsdf evaluation
					result += throughput * throughputs[i] * bsdf->eval(bRec) * std::abs(Frame::cosTheta(bRec.wo));
			}

			// 5. Recursively sample indirect illumination
			// = use current object's bsdf to change the throughput of
			//   future contributions
			// = stop here if the throughput is null
			BSDFQueryRecord bRec(its.toLocal(-ray.d));
			Color3f bsdfWeight = bsdf->sample(bRec, sampler->next2D());
			if ((bsdfWeight.array() == 0).all())
				break;
			eta *= bRec.eta;
			throughput *= bsdfWeight;
			// if the relative index got too small or too big, we stop
			if (eta > 2 || eta < 0.5) {
				cout << "OOps!" << endl;
				break;
			}
			// + generate the new ray!
			ray = Ray3f(its.p, its.shFrame.toWorld(bRec.wo));

			// we include the next bounce emitted radiance
			// only if the previous one had a zero pdf which
			// made its direct lighting component zero
			// => decreases the variance because
			//    hitting a light is a really unlikely event
			includeEmitted = bRec.measure == EDiscrete;

			// 6. Apply Russian Roulette after the main bounces (which we want to keep)
			if (++depth > 1) {
				/* Russian roulette: try to keep path weights equal to one,
				 while accounting for the radiance change at refractive index
				 boundaries. Stop with at least some probability to avoid
				 getting stuck (e.g. due to total internal reflection) */
				if (sampler->next1D() >= probabilty_to_continue_eye)
					break;
				throughput /= probabilty_to_continue_eye;
			}
		}

		return result;
	}

	QString toString() const {
		return "PathTracer[]";
	}

};

GROUP_NAMESPACE_END

NORI_REGISTER_GROUP_CLASS(PathTracer, "path");
NORI_NAMESPACE_END
