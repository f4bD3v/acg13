


#include <nori/bsdf.h>
#include <nori/frame.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Dielectric BRDF model
 */
class Dielectric : public BSDF {
public:
	Dielectric(const PropertyList &propList) {
		m_eta_i = propList.getFloat("eta_i",1.0f); //default refractive index of air
		m_eta_t = propList.getFloat("eta_t",1.5f); //default refractive index of glas
		mColor = propList.getColor("color",Color3f(0.5f));
	}

	/// Reflection in local coordinates
	inline Vector3f reflect(const Vector3f &wi) const {
		return Vector3f(-wi.x(), -wi.y(), wi.z());
	}

	/// Compute norm of vector
	inline float norm(const Vector3f &v) const{
		return sqrt(pow(v.x(),2)+pow(v.y(),2)+pow(v.z(),2));
	}

    /// Clamp value between lower and upper bound
	inline float clamp(const float val, float low, float high) const{
		if(val<low) return low;
		else if(val>high) return high;
		else return val;
	}

	/// Evaluate the BRDF model
	Color3f eval(const BSDFQueryRecord &bRec) const {
		float cos_theta_i = clamp(Frame::cosTheta(bRec.wi),-1,1);
		//The BRDF is not smooth and illumation from backside is possible.
		if (cos_theta_i == 0)
			return Color3f(0.0f);
			
		//Compute the Fresnel Reflectance = 1-Fresnel Transmittance
		// This is the ratio of the the light that is reflected
		float F_r = fresnel(cos_theta_i, m_eta_i, m_eta_t);


		//Compute the refracted direction with Snell's law and substract the outgoing direction of
		// the bRec. If the difference is small enough, we have refraction
		Vector3f w = refract(bRec.wi, cos_theta_i, m_eta_i, m_eta_t) - bRec.wo;
		if ((w.array() <= 1e-3 && w.array() >= -1e-3).all()) {
			//Compute relative index of refraction = IOR_entering/IOR_transmitting
			float eta = m_eta_i/m_eta_t;
			//If ray is coming from inside material, the index must be inverted
			if (cos_theta_i < 0) {
				eta = 1.0f/eta;
				//We want the absolute value of the cosine value
				cos_theta_i = -cos_theta_i;
			}
			// Return BRDFvalue for refraction
			return mColor * (1.0f-F_r) / (eta*eta*cos_theta_i);
		} else {
			// If no refraction, compute the reflected directon with Snell's law and substract
			// the outgoing direction of the bRec. If the difference is small enough, we have
			// reflection
			w = reflect(bRec.wi) - bRec.wo;
			if ((w.array() <= 1e-3 && w.array() >= -1e-3).all()) {
				// Return the BRDFvalue for reflection
				return mColor * F_r / std::abs(Frame::cosTheta(bRec.wo));
			}
		}
		//If neither refraction or reflection, the BRDF is zero.
		//The BRDF-function is discrete with only two nonzero directions.
		return Color3f(0.0f);
	}

	/// Compute the density of \ref sample() wrt. solid angles
	float pdf(const BSDFQueryRecord &bRec) const {
		//We have a discrete BRDF-function. There is no chance distribution.
		return 1.0f;
	}

	/// Draw a a sample from the BRDF model
	Color3f sample(BSDFQueryRecord &bRec, const Point2f &sample) const {
		float cos_theta_i = clamp(Frame::cosTheta(bRec.wi), -1, 1);
		// Illumination from backside is possible, we only return zero if the incoming angle's cosine
		// is zero.
		if (cos_theta_i == 0)
			return Color3f(0.0f);

		// Set bRec parameters
		bRec.eta = 1.0f;
		bRec.measure = EDiscrete;
		//Compute the Fresnel Reflectance = 1-Fresnel Transmittance
		// This is the ratio of the the light that is reflected
		// and thus also the probability that light is reflected
		float F_r = fresnel(cos_theta_i, m_eta_i, m_eta_t);

		//A random sample decides whether the ray is reflected or refracted
		if (sample.x() < F_r) {
			// Reflection:
			// Compute the reflected direction with Snell's law and return color of
			// material weighted with Fresnel Reflectance
			bRec.wo = reflect(bRec.wi);
			return mColor * F_r;
		} else {
			//Refraction:
			// Compute the refracted direction with Snell's law
			bRec.wo = refract(bRec.wi, cos_theta_i, m_eta_i, m_eta_t);
			//Compute relative index of refraction = IOR_entering/IOR_transmitting
			float eta = m_eta_i/m_eta_t;
			if (cos_theta_i < 0) {
				//If ray is coming from inside material, the index must be inverted
				eta = 1.0f/eta;
				//We want the absolute value of the cosine value
				cos_theta_i = -cos_theta_i;
			}
			return mColor * (1.0f - F_r) * std::abs(Frame::cosTheta(bRec.wo))
						  / (eta * eta * cos_theta_i);
		}
	}

	/// Return a human-readable summary
	QString toString() const {
		return QString(
			"Dielectric[\n"
			" eta_i = %1\n"
			" eta_t = %2\n"
			" color = %3\n"
			"]").arg(m_eta_i).arg(m_eta_t).arg(mColor.toString());
	}

		Color3f getColor() const { return mColor; }

	EClassType getClassType() const { return EBSDF; }


private:
	float m_eta_i, m_eta_t;
	Color3f mColor;
};

NORI_REGISTER_CLASS(Dielectric, "dielectric");
NORI_NAMESPACE_END
