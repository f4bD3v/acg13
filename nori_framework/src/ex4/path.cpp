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
#define probability_to_continue_eye 0.8
#define probability_to_continue_light 0.9
#define max_eye_its 1000
#define max_light_its 1000

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

	/// Return a direction in the hemisphere defined by normal
	inline const Vector3f getDirection(Normal3f &normal, Sampler *sampler) const {
		Vector3f direction = squareToUniformSphere(sampler->next2D());
		if (direction.dot(normal) < 0)
			direction = -direction;
		return direction.normalized();
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
	 * \param size
	 * size of the luminaires used
	 *
	 * \return the sampled light radiance including its geometric, visibility and pdf weights
	 */
	inline Color3f sampleLight(const Scene *scene, LuminaireQueryRecord &lRec, int size) const {
		// Compute distance between the two points (from first mesh, to luminaire mesh)
		lRec.d = lRec.p - lRec.ref;
		float dist2 = lRec.d.squaredNorm();
		lRec.dist = std::sqrt(dist2);
		lRec.d /= lRec.dist;

		// Correct side of luminaire
		// /!\ if on the wrong side, then we get no contribution!
		float dp = -lRec.n.dot(lRec.d);
		if (dp > 0) {
			// Check the visibility
			if (scene->rayIntersect(Ray3f(lRec.ref, lRec.d, Epsilon, lRec.dist * (1 - 1e-4f))))
				return Color3f(0.0f);
			Color3f value = lRec.luminaire->getColor();
			return value * dp * size  / dist2 / getMesh(lRec.luminaire)->pdf();
		} else {
			// wrong side of luminaire!
			return Color3f(0.0f);
		}
	}

	Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &_ray) const {
		float eta = 1.0f;
		// LIGHT PATH
		std::vector<Intersection> itsL;
		std::vector<Color3f> throughputs;
		// 1. Choose a random light
		const std::vector<Luminaire *> &luminaires = scene->getLuminaires();
		int index = std::min((int) (luminaires.size() * sampler->next1D()), (int) luminaires.size() - 1);
		const Luminaire *luminaire_path = luminaires[index];
		// 2. Choose a random point in the light
		const Mesh *mesh = getMesh(luminaire_path);
		Normal3f normal_path;
		itsL.push_back(Intersection());
		mesh->samplePosition(sampler->next2D(), itsL[0].p, normal_path);
		// 3. Choose a random direction in the same half plane as the normal
		const Vector3f direction = getDirection(normal_path, sampler);
		// 4. Create the ray form the light in the random direction
		Ray3f rayL = Ray3f(itsL[0].p, direction);
		// 5. Push initial throughput
		throughputs.push_back(Color3f(1.0f));
		// 6. Compute the light path
		unsigned int real_length = 1;
		Color3f bsdfWeight = Color3f(1.0f);
		cout << "light is at:\n" << itsL[0].p << "\n";
		while (real_length < max_light_its) {
			// test russian roulette
			if (sampler->next1D() >= probability_to_continue_light)
				break;
			// add new intersection and throughput
			itsL.push_back(Intersection());
			throughputs.push_back(Color3f(1.0f));
			// 6.a. Compute next intersection
			if (!scene->rayIntersect(rayL, itsL[real_length]))
				break;
			if (real_length == 1) {
				Vector3f vec = itsL[real_length-1].p - itsL[real_length].p;
				float d = std::sqrt(vec.squaredNorm());
				vec /= d;
				throughputs[1] = luminaire_path->getColor() * luminaires.size()
								 * INV_PI * scene->evalTransmittance(Ray3f(itsL[1].p, vec, 0, d), sampler)
								 * std::abs(Frame::cosTheta(itsL[real_length].toLocal(vec)))
								 / probability_to_continue_light;
			} else {
				// 6.b. Update throughput
				//Vector3f vec = itsL[real_length-1].p - itsL[real_length].p;
				//float d = std::sqrt(vec.squaredNorm());
				//vec /= d;
				throughputs[real_length] *= throughputs[real_length-1]
											* bsdfWeight / probability_to_continue_light
											;//* std::abs(Frame::cosTheta(itsL[real_length].toLocal(vec)));
			}
			BSDFQueryRecord bRec(itsL[real_length].toLocal(-rayL.d));
			const BSDF *bsdf = itsL[real_length].mesh->getBSDF();
			cout << "Let's hope... " << real_length << "\n";
			bsdfWeight = bsdf->sample(bRec, sampler->next2D());
			cout << bsdfWeight << "\n";
			if ((bsdfWeight.array() == 0).all())
				break;
			eta *= bRec.eta;
			// if the relative index got too small or too big, we stop
			if (eta > 2 || eta < 0.5) {
				cout << "OOps!" << endl;
				break;
			}
			// 6.c. Generate the new ray
			rayL = Ray3f(itsL[real_length].p, itsL[real_length].shFrame.toWorld(bRec.wo));
			cout << "Ray.o:" << rayL.o << "\n";
			cout << "Ray.d:" << rayL.d << "\n\n";
			// 6.d. Update length
			++real_length;
		}


		// EYE PATH
		Ray3f ray(_ray);
		Intersection its;
		Color3f result(0.0f), throughput(1.0f);
		int depth = 1;
		// whether to use the emitted light we would hit
		// - first hit => yes
		// - hit through a mirror => yes
		// - hit through something else => no (direct lighting already takes it into account)
		bool includeEmitted = true;
		while (depth < max_eye_its) {
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

			for (unsigned int i = 0; i < real_length; ++ i) {
				Point3f p = itsL[i].p - its.p;
				if ((p.array() <= 1e-3 && p.array() >= -1e-3).all()) {
cout << "BEAUTIFUL !!!!!!!!!!!!!!!!!!\n\n";
					result += throughput * throughputs[i]
					/ (i + depth);
				}
			}

			// 3. Direct illumination sampling
			LuminaireQueryRecord lRec(luminaire_path, its.p, itsL[0].p, normal_path);
			Color3f direct = sampleLight(scene, lRec, luminaires.size());
			if ((direct.array() != 0).any()) {
				// Yay !
				// Update result by combining the throughputs and the bsdf evaluation
				BSDFQueryRecord bRec(its.toLocal(-ray.d),
									 its.toLocal(lRec.d), ESolidAngle);
				result += throughput * direct * bsdf->eval(bRec)
				* scene->evalTransmittance(Ray3f(lRec.ref, lRec.d, 0, lRec.dist), sampler)
				* std::abs(Frame::cosTheta(bRec.wo))
				/ (depth);
			}

			// 4. Combine eye and light paths
			// try to combine the current its to each point of the light path
			// except the first point which is on the light
			for (unsigned int i = 1; i < real_length; ++i) {
				// test if obstacle in the way
				Vector3f bRec1_wi = its.p - itsL[i].p;
				float dist = std::sqrt(bRec1_wi.squaredNorm());
				bRec1_wi /= dist;
				if (!scene->rayIntersect(Ray3f(itsL[i].p, bRec1_wi, Epsilon, dist * (1 - 1e-4f)))) {
					// Yay !
					// Update result by combining the throughputs and the bsdf evaluation
					BSDFQueryRecord bRec1(itsL[i].toLocal(bRec1_wi),
										  itsL[i].toLocal((itsL[i-1].p - itsL[i].p).normalized()), ESolidAngle);
					BSDFQueryRecord bRec(its.toLocal(-ray.d),
										 its.toLocal(-bRec1_wi), ESolidAngle);
					if (bsdf->pdf(bRec) != 0) {
						const BSDF *bsdf1 = itsL[i].mesh->getBSDF();
//cout << "throughput\n" << throughput << "\n";
//cout << "eval1\n" << bsdf1->eval(bRec1) << "\n";
//cout << "eval\n" << bsdf->eval(bRec) << "\n";
//cout << "throughputs\n" << throughputs[i] << "\n\n";
						result += throughput * throughputs[i]
								* bsdf1->eval(bRec1) * std::abs(Frame::cosTheta(bRec1.wo))
								* bsdf->eval(bRec)   * std::abs(Frame::cosTheta(bRec.wo))
								/ (i + 1 + depth);
					}
				}
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
			if (++depth > 2) {
				/* Russian roulette: try to keep path weights equal to one,
				 while accounting for the radiance change at refractive index
				 boundaries. Stop with at least some probability to avoid
				 getting stuck (e.g. due to total internal reflection) */
				if (sampler->next1D() >= probability_to_continue_eye)
					break;
				throughput /= probability_to_continue_eye;
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
