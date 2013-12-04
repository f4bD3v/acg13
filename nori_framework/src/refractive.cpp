



#include <nori/bsdf.h>
#include <nori/frame.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Pure refractive BRDF model
 */
class Refractive : public BSDF {
public:
    Refractive(const PropertyList &propList) {
        m_eta_i = propList.getFloat("eta_i",1.0f); //default refractive index of air
        m_eta_t = propList.getFloat("eta_t",1.5f); //default refractive index of glas
		mColor = propList.getColor("color",Color3f(0.5f));
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
        if (bRec.measure != ESolidAngle ||
                Frame::cosTheta(bRec.wi)==0 || Frame::cosTheta(bRec.wo)==0)
            return Color3f(0.0f);
        float eta = m_eta_i/m_eta_t;
		float cos_theta_i = clamp(Frame::cosTheta(bRec.wi),-1,1);
		
        Vector3f wt = refract(bRec.wi, cos_theta_i, m_eta_i, m_eta_t);
		
        if (cos_theta_i < 0){
            eta = 1/eta;
			cos_theta_i = -cos_theta_i;
		}

        Color3f result =  1/(eta*eta*cos_theta_i) * mColor;
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
		
		
        Vector3f wt = refract(bRec.wi, cos_theta_i, m_eta_i, m_eta_t);
        //std::cout << "wi = " << bRec.wi << std::endl;
        //std::cout << "wt " << wt << std::endl;
		
		if (cos_theta_i < 0){
            eta = 1/eta;
		}

        bRec.measure = ESolidAngle;

        
        bRec.wo = wt; 
        bRec.eta = 1.0f;
        float cos = std::abs(wt.z());
		
       
        Color3f result = eval(bRec)*cos/pdf(bRec);
		return result;
    }

    /// Return a human-readable summary
    QString toString() const {
        return QString(
            "Refractive[\n"
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

NORI_REGISTER_CLASS(Refractive, "refractive");
NORI_NAMESPACE_END
