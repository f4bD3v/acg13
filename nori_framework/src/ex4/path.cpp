/*******************************************************************************
 *  path.cpp
 *******************************************************************************
 *  Copyright (c) 2013 Alexandre Kaspar <alexandre.kaspar@a3.epfl.ch>
 *  For Advanced Computer Graphics, at the LGG / EPFL
 *
 *        DO NOT REDISTRIBUTE
 ***********************************************/

#include <nori/acg.h>
#include <nori/block.h>
#include <nori/camera.h>
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
#define max_eye_points 1000
#define max_light_points 1000

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

	/**
	 * @brief Li
	 * @param scene
	 * @param sampler
	 * @param _ray
	 * @param light_image
	 * @return
	 */
	Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &_ray, ImageBlock *light_image) const {
		float eta = 1.0f;
		// ================================================
		// ================== LIGHT PATH ==================
		// ================================================
		std::vector<Intersection> itsL;
		std::vector<Color3f> throughputs;
		unsigned int real_length = 0;
		int luminaires_size;
		Normal3f normal_path;
		const Luminaire *luminaire_path;
		if (max_light_points > 0) {
			// 1. Choose a random light
			const std::vector<Luminaire *> &luminaires = scene->getLuminaires();
			luminaires_size = (int) luminaires.size();
			int index = std::min((int) (luminaires_size * sampler->next1D()), luminaires_size - 1);
			luminaire_path = luminaires[index];

			// 2. Choose a random point in the light
			const Mesh *mesh = getMesh(luminaire_path);
			itsL.push_back(Intersection());
			mesh->samplePosition(sampler->next2D(), itsL[0].p, normal_path);

			// 3. Choose a random direction in the same half plane as the normal
			const Vector3f direction = getDirection(normal_path, sampler);
			// 4. Create the ray form the light in the random direction
			Ray3f rayL = Ray3f(itsL[0].p, direction);
			// 5. Push initial throughput
			throughputs.push_back(luminaire_path->getColor() * luminaires.size());

			// 6. Compute the light path
			real_length = 1;
			Color3f bsdfWeight = Color3f(1.0f);
			Vector3f wi;
			while (real_length < max_light_points) {
				// 7. Check eye path of length 0
				//    Try to connect itsL[real_length-1] to the eye
				//    Find concerned pixel (most probably not the current one)
				//    Update light_image and NOT RESULT !
				// 7.a. Create vector from last its to eye
				Vector3f vec = itsL[real_length-1].p - _ray.o;
				float dist = std::sqrt(vec.squaredNorm());
				vec /= dist;
				Ray3f ray_pixel(_ray.o, vec, Epsilon, dist * (1 - 1e-7f));
				// 7.b. Check visibility
				if (!scene->rayIntersect(ray_pixel)) {
					// 7.c. Get camera
					const Camera *camera = scene->getCamera();
					// 7.d. Compute weight for this contribution
					/*float w = (float)(NORI_BLOCK_SIZE * NORI_BLOCK_SIZE)
							  / (float)(real_length * sampler->getSampleCount()
										* camera->getOutputSize().x() * camera->getOutputSize().y());
					*/float w = 1.0f/real_length;
					// 7.e. Update concerned pixel in light_image
					if (real_length == 1)  {
						light_image->lock();
						light_image->put(camera->getPixel(ray_pixel), throughputs[0] * w);
						light_image->unlock();
					} else {
						BSDFQueryRecord bRec(wi, itsL[real_length-1].toLocal(-ray_pixel.d), ESolidAngle);
						light_image->lock();
						light_image->put(camera->getPixel(ray_pixel),
									 throughputs[real_length-1] * w
									 * itsL[real_length-1].mesh->getBSDF()->eval(bRec)
									 * std::abs(Frame::cosTheta(bRec.wo)));
						light_image->unlock();
					}
				} else {
					light_image->lock();
					//light_image->put(Point2f(10.0f, 100.0f), Color3f(0.0f));
					light_image->unlock();
				}

				// test russian roulette
				if (sampler->next1D() >= probability_to_continue_light)
					break;
				// add new intersection and throughput
				itsL.push_back(Intersection());
				throughputs.push_back(Color3f(1.0f));
				// 8. Compute next intersection
				if (!scene->rayIntersect(rayL, itsL[real_length]))
					break;

				// 9. Update throughput
				if (real_length == 1) {
					Vector3f vec = itsL[0].p - itsL[1].p;
					float d = std::sqrt(vec.squaredNorm());
					vec /= d;
					d = -normal_path.dot(vec);
					if (d > 0) {
						throughputs[1] = throughputs[0] * INV_PI
									 	* scene->evalTransmittance(Ray3f(itsL[1].p, vec, 0, d), sampler)
									 	* d
									 	/ probability_to_continue_light;
					}
				} else {
					throughputs[real_length] *= throughputs[real_length-1] * bsdfWeight / probability_to_continue_light;
				}

				// 10. sample the bsdf for the new direction
				BSDFQueryRecord bRec(itsL[real_length].toLocal(-rayL.d));
				wi = bRec.wi;
				const BSDF *bsdf = itsL[real_length].mesh->getBSDF();
				bsdfWeight = bsdf->sample(bRec, sampler->next2D());
				if ((bsdfWeight.array() == 0).all())
					break;
				eta *= bRec.eta;
				// if the relative index got too small or too big, we stop
				if (eta > 2 || eta < 0.5) {
					cout << "OOps!" << endl;
					break;
				}

				// 11. Generate the new ray
				rayL = Ray3f(itsL[real_length].p, itsL[real_length].shFrame.toWorld(bRec.wo));
				// 12. Update length
				++real_length;
			}
		}

		// ==============================================
		// ================== EYE PATH ==================
		// ==============================================
		Intersection its;
		Ray3f ray(_ray);
		Color3f result(0.0f), throughput(1.0f);
		int depth = 1;
		// whether to use the emitted light we would hit
		// - first hit => yes
		// - hit through a mirror => yes
		// - hit through something else => no (direct lighting already takes it into account)
		bool includeEmitted = true;
		while (depth < max_eye_points) {
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

			// 2Bis. Check if any intersection of light path is the same as current its
			for (unsigned int i = 0; i < real_length; ++ i) {
				Point3f p = itsL[i].p - its.p;
				if ((p.array() <= 1e-3 && p.array() >= -1e-3).all()) {
					result += throughput * throughputs[i] / (i + depth);
				}
			}

			// 3. Direct illumination sampling
			// if light path has length 0
			LuminaireQueryRecord lRec(its.p);
			Color3f direct = sampleLights(scene, lRec, sampler->next2D());
			// if not (most of the time)
			if (max_light_points != 0) {
				lRec = LuminaireQueryRecord(luminaire_path, its.p, itsL[0].p, normal_path);
				direct = sampleLight(scene, lRec, luminaires_size);
			}
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
					const BSDF *bsdf1 = itsL[i].mesh->getBSDF();
					result += throughput * throughputs[i]
							* bsdf1->eval(bRec1) * std::abs(Frame::cosTheta(bRec1.wo))
							* bsdf->eval(bRec)   * std::abs(Frame::cosTheta(bRec.wo))
							/ (i + 1 + depth);
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
