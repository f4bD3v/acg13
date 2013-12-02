


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
        /* This is a smooth BRDF -- return zero if the measure
           is wrong */
        if (bRec.measure != ESolidAngle || Frame::cosTheta(bRec.wi)==0
                    || Frame::cosTheta(bRec.wo)==0)
            return Color3f(0.0f);
        float eta = m_eta_i/m_eta_t;
		float cos_theta_i = clamp(Frame::cosTheta(bRec.wi),-1,1);
		
        float F_r = fresnel(cos_theta_i, m_eta_i, m_eta_t);
		Vector3f wo = reflect(bRec.wi);
        Vector3f wt = refract(bRec.wi, cos_theta_i, m_eta_i, m_eta_t);
		
        if (cos_theta_i < 0){
            eta = 1/eta;
			cos_theta_i = -cos_theta_i;
		}

		Color3f result;
		if(bRec.wo == wo){
            result= F_r*mColor/cos_theta_i;
		}
		else if(bRec.wo == wt){
            result= mColor/(eta*eta) * (1.0f-F_r)/cos_theta_i;
		}
		else{
			result= Color3f(0.0f);
		}
		
		return result;
    }

    /// Compute the density of \ref sample() wrt. solid angles
    float pdf(const BSDFQueryRecord &bRec) const {

		return 1.0f;
    }

    /// Draw a a sample from the BRDF model
    Color3f sample(BSDFQueryRecord &bRec, const Point2f &sample) const {
        float eta = m_eta_i/m_eta_t;
		float cos_theta_i = clamp(Frame::cosTheta(bRec.wi),-1,1);
		if(cos_theta_i==0)
            return Color3f(0.0f);
		
		
        float F_r = fresnel(cos_theta_i, m_eta_i, m_eta_t);
		Vector3f wo = reflect(bRec.wi);
        Vector3f wt = refract(bRec.wi, cos_theta_i, m_eta_i, m_eta_t);
		
		if (cos_theta_i < 0){
            eta = 1/eta;
		}

        bRec.measure = ESolidAngle;

        //Select reflection or refraction
        bool useReflection = true;
        if (sample.x() > F_r) {
            useReflection = false;
        }

        float cos;
		Color3f f_r;

        if(useReflection){
            bRec.wo = wo;
            bRec.eta = 1.0f;
            cos = std::abs(Frame::cosTheta(bRec.wo));
        }
        else {
            bRec.wo = wt; 
            //bRec.eta = eta;
            bRec.eta = 1.0f;
			cos = std::abs(wt.z());
        }

 	   
        return eval(bRec)*cos/pdf(bRec);
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
