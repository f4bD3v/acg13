


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

	inline float clamp(const float val, float low, float high) const{
		if(val<low) return low;
		else if(val>high) return high;
		else return val;
	}

	/// Evaluate the BRDF model
	Color3f eval(const BSDFQueryRecord &bRec) const {
		float cos_theta_i = clamp(Frame::cosTheta(bRec.wi),-1,1);
		if (cos_theta_i == 0)
			return Color3f(0.0f);

		Vector3f w = refract(bRec.wi, cos_theta_i, m_eta_i, m_eta_t) - bRec.wo;
		if ((w.array() <= 1e-3 && w.array() >= -1e-3).all()) {
			float eta = m_eta_i/m_eta_t;
			if (cos_theta_i < 0) {
				eta = 1.0f/eta;
				cos_theta_i = -cos_theta_i;
			}
			float F_r = fresnel(cos_theta_i, m_eta_i, m_eta_t);
			return mColor * (1.0f-F_r) / (eta*eta*cos_theta_i);
		} else {
			w = reflect(bRec.wi) - bRec.wo;
			if ((w.array() <= 1e-3 && w.array() >= -1e-3).all()) {
				float F_r = fresnel(cos_theta_i, m_eta_i, m_eta_t);
				return mColor * F_r / std::abs(Frame::cosTheta(bRec.wo));
			}
		}
		return Color3f(0.0f);
	}

	/// Compute the density of \ref sample() wrt. solid angles
	float pdf(const BSDFQueryRecord &bRec) const {
		return 1.0f;
	}

	/// Draw a a sample from the BRDF model
	Color3f sample(BSDFQueryRecord &bRec, const Point2f &sample) const {
		float cos_theta_i = clamp(Frame::cosTheta(bRec.wi), -1, 1);
		if (cos_theta_i == 0)
			return Color3f(0.0f);

		bRec.eta = 1.0f;
		bRec.measure = EDiscrete;
		float F_r = fresnel(cos_theta_i, m_eta_i, m_eta_t);

		if (sample.x() < F_r) {
			bRec.wo = reflect(bRec.wi);
			return mColor * F_r;
		} else {
			bRec.wo = refract(bRec.wi, cos_theta_i, m_eta_i, m_eta_t);
			float eta = m_eta_i/m_eta_t;
			if (cos_theta_i < 0) {
				eta = 1.0f/eta;
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
