


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
        m_eta_t = propList.getFloat("eta_t",1.5f); //default refractive index of glass
    }

    /// Reflection in local coordinates
    inline Vector3f reflect(const Vector3f &wi) const {
        return Vector3f(-wi.x(), -wi.y(), wi.z());
    }

    /// Evaluate the BRDF model
    Color3f eval(const BSDFQueryRecord &bRec) const {
        /* This is a smooth BRDF -- return zero if the measure
           is wrong, or when queried for illumination on the backside */
        if (bRec.measure != ESolidAngle
            || Frame::cosTheta(bRec.wi) <= 0
            || Frame::cosTheta(bRec.wo) <= 0)
            return Color3f(0.0f);

        /*TODO*/
    }

    /// Compute the density of \ref sample() wrt. solid angles
    float pdf(const BSDFQueryRecord &bRec) const {
        /* This is a smooth BRDF -- return zero if the measure
           is wrong, or when queried for illumination on the backside */
        if (bRec.measure != ESolidAngle
            || Frame::cosTheta(bRec.wi) <= 0
            || Frame::cosTheta(bRec.wo) <= 0)
            return 0.0f;


        /*TODO*/
    }

    /// Draw a a sample from the BRDF model
    Color3f sample(BSDFQueryRecord &bRec, const Point2f &sample) const {
        float eta = m_eta_i/m_eta_t;
        if (Frame::cosTheta(bRec.wi) <= 0)
            eta = 1/eta;


        bRec.measure = ESolidAngle;

        // Compute reflective and refractive angle
        Vector3f wo=reflect(bRec.wi);   //TODO check for total internal reflection
        Vector3f wt= 1/eta * wo - (Frame::cosTheta(bRec.wi))*(1-1/eta)*Frame.n;
        float cos_theta_t = sqrt(1-1/pow(eta,2)*(1-pow(Frame::cosTheta(bRect.wi),2)));

        //Compute Fresnel reflectance
        float r_par = (Frame::cosTheta(bRec.wi)-eta*cos_theta_t)/(Frame::cosTheta(bRec.wi)+eta*cost_theta_t);
        float r_perp = (eta*Frame::cosTheta(bRec.wi)-cos_theta_t)/(eta*Frame::cosTheta(bRec.wi)+cos_theta_t);
        float F_r = 0.5f*(pow(r_par,2)+pow(r_perp,2));

        //Select reflection or refraction
        bool useReflection = true;
        if (sample.x() > F_r) {
            useReflection = false;
        }

        if(useReflection){
            bRec.wo = wo;
            bRec.eta = 1.0f;
            return eval(bRec)*Frame::cosTheta(bRec.wo)/pdf(bRec);
        }
        else {
            bRec.wo = wt; //TODO check that path tracer accepts rays through materials
            bRec.eta = eta;
            return eval(bRec)*cost_theta_t/pdf(bRec);
        }

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

        Color3f getColor() const { /*TODO*/ }

    EClassType getClassType() const { return EBSDF; }
private:
    float m_eta_i, m_eta_t;
};

NORI_REGISTER_CLASS(Dielectric, "dielectric");
NORI_NAMESPACE_END
