


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
    }

    /// Reflection in local coordinates
    inline Vector3f reflect(const Vector3f &wi) const {
        return Vector3f(-wi.x(), -wi.y(), wi.z());
    }

    /// Compute norm of vector
    inline float norm(const Vector3f &v) const{
        return sqrt(pow(v.x(),2)+pow(v.y(),2)+pow(v.z(),2));
    }

    /// Evaluate the BRDF model
    Color3f eval(const BSDFQueryRecord &bRec) const {
        /* This is a smooth BRDF -- return zero if the measure
           is wrong */
        if (bRec.measure != ESolidAngle || Frame::cosTheta(bRec.wi)==0)
            return Color3f(0.0f);
        float eta = m_eta_i/m_eta_t;
        if (Frame::cosTheta(bRec.wi) < 0)
            eta = 1/eta;

        // Compute reflective and refractive angle
        Vector3f wo=reflect(bRec.wi);
        Vector3f normal = (wo-bRec.wi)/norm(wo-bRec.wi);
        Vector3f n(0,1,0);
        float sin_theta_t = eta*sqrt(1.0f-pow(Frame::cosTheta(bRec.wi),2));
        float cos_theta_t = sqrt(1.0f-pow(sin_theta_t,2));
        Vector3f wt= 1/eta * wo - (cos_theta_t-Frame::cosTheta(bRec.wi)/eta)*n;
        //Vector3f wt= 1/eta * wo - (Frame::cosTheta(bRec.wi))*(1-1/eta)*normal;

        //Compute Fresnel reflectance
		
        float r_par = (Frame::cosTheta(bRec.wi)-eta*cos_theta_t)/(Frame::cosTheta(bRec.wi)+eta*cos_theta_t);
        float r_perp = (eta*Frame::cosTheta(bRec.wi)-cos_theta_t)/(eta*Frame::cosTheta(bRec.wi)+cos_theta_t);

        Color3f F_r = Color3f(0.5f*(pow(r_par,2)+pow(r_perp,2)));



		
        //Handle total internal reflection
        if(sin_theta_t>1){
			F_r = Color3f(1.0f);
        }

		if(bRec.wo == wo){
            return F_r/Frame::cosTheta(bRec.wi);
		}
		else if(bRec.wo == wt){
            return 1/pow(eta,2)*(1.0f-F_r)/Frame::cosTheta(bRec.wi);
		}
		else{
			return Color3f(0.0f);
		}
    }

    /// Compute the density of \ref sample() wrt. solid angles
    float pdf(const BSDFQueryRecord &bRec) const {
        /* This is a smooth BRDF -- return zero if the measure
           is wrong */
        if (bRec.measure != ESolidAngle)
            return 0.0f;

		return 1.0f;
    }

    /// Draw a a sample from the BRDF model
    Color3f sample(BSDFQueryRecord &bRec, const Point2f &sample) const {
        float eta = m_eta_i/m_eta_t;
        if (Frame::cosTheta(bRec.wi) < 0)
            eta = 1/eta;
		else if(Frame::cosTheta(bRec.wi)==0)
            return Color3f(0.0f);


        bRec.measure = ESolidAngle;

        // Compute reflective and refractive angle
        Vector3f wo=reflect(bRec.wi);
        Vector3f normal = (wo-bRec.wi)/norm(wo-bRec.wi);
        Vector3f n(0,1,0);
        float cos_theta_t = sqrt(1-1/pow(eta,2)*(1-pow(Frame::cosTheta(bRec.wi),2)));
        //Vector3f wt= 1/eta * wo - (Frame::cosTheta(bRec.wi))*(1-1/eta)*normal;
        Vector3f wt= 1/eta * wo - (cos_theta_t-Frame::cosTheta(bRec.wi)/eta)*n;

        //Compute Fresnel reflectance
        float r_par = (Frame::cosTheta(bRec.wi)-eta*cos_theta_t)/(Frame::cosTheta(bRec.wi)+eta*cos_theta_t);
        float r_perp = (eta*Frame::cosTheta(bRec.wi)-cos_theta_t)/(eta*Frame::cosTheta(bRec.wi)+cos_theta_t);
        float F_r = 0.5f*(pow(r_par,2)+pow(r_perp,2));
		
        //Handle total internal reflection
        float sin_theta_t = sqrt(1-pow(cos_theta_t,2));
        if(sin_theta_t>1){
			F_r = 1.0f;
        }

        //Select reflection or refraction
        bool useReflection = true;
        if (sample.x() > F_r) {
            useReflection = false;
        }

        float cos;

        if(useReflection){
            bRec.wo = wo;
            bRec.eta = 1.0f;
            cos = Frame::cosTheta(bRec.wo);
        }
        else {
            bRec.wo = wt; //TODO check that path tracer accepts rays through materials
            bRec.eta = eta;
            cos = cos_theta_t;
        }
		
        //std::cout << "fr = " << eval(bRec) << std::endl;
        //std::cout << "cos = " << cos << std::endl;
        //std::cout << "pdf = " << pdf(bRec) << std::endl;
        //std::cout << "result = " << eval(bRec)*cos/pdf(bRec) << std::endl;
        return eval(bRec)*cos/pdf(bRec);

        /*Possible issues with implementation:
         *  invalid values for cosine
         *  rest of framework doesn't accept rays through materials
         *  total internal reflection
         */
    }

    /// Return a human-readable summary
    QString toString() const {
        return QString(
            "Dielectric[\n"
            " eta_i = %1\n"
            " eta_t = %2\n"
            "]").arg(m_eta_i,m_eta_t);
    }

        Color3f getColor() const { return m_eta_t; }

    EClassType getClassType() const { return EBSDF; }


private:
    float m_eta_i, m_eta_t;
};

NORI_REGISTER_CLASS(Dielectric, "dielectric");
NORI_NAMESPACE_END
