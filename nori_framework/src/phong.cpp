/*******************************************************************************
 *  phong.cpp
 *******************************************************************************
 *  Copyright (c) 2013 Alexandre Kaspar <alexandre.kaspar@a3.epfl.ch>
 *  For Advanced Computer Graphics, at the LGG / EPFL
 *
 *        DO NOT REDISTRIBUTE
 ***********************************************/

#include <nori/bsdf.h>
#include <nori/common.h>
#include <nori/frame.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Phong BRDF model
 */
class Phong : public BSDF {
public:

	Phong(const PropertyList &propList) {
		m_Kd = propList.getColor("kd", Color3f(0.5f));
		m_Ks = propList.getColor("ks", Color3f(0.5f));
		m_exp = propList.getFloat("n", 20.0f);
		// computation of the sampling weights
		float wd = m_Kd.getLuminance();
		float ws = m_Ks.getLuminance();
		m_specSamplingWeight = ws / (ws + wd);
		m_diffSamplingWeight = 1.0f - m_specSamplingWeight;
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
			{
						 //std::cout << "! phong.cpp line 47" << std::endl;
						 //if(Frame::cosTheta(bRec.wi)<=0)
							//std::cout << "wi! "<< std::endl;
						 //else if(Frame::cosTheta(bRec.wo)<=0)
							 //std::cout << "wo! " << std::endl;
						 return Color3f(0.0f);
			 }

		// Based on http://www.cs.virginia.edu/~jdl/importance.doc‎

		// reflection of input => best output
		Vector3f bestDir = reflect(bRec.wi);

		/* The BRDF is given by
				Kd / pi
			  + Ks (n+2)/2pi (cos alpha)^n
		 */

		float cos_alpha = std::max(0.0f, bestDir.dot(bRec.wo)); // clamp angle to pi/2
		return (m_Kd + m_Ks * 0.5f * (m_exp + 2.0f) * std::pow(cos_alpha, m_exp)) * INV_PI;
	}

	/// Compute the density of \ref sample() wrt. solid angles

	float pdf(const BSDFQueryRecord &bRec) const {
		/* This is a smooth BRDF -- return zero if the measure
		   is wrong, or when queried for illumination on the backside */
		if (bRec.measure != ESolidAngle
				|| Frame::cosTheta(bRec.wi) <= 0
				|| Frame::cosTheta(bRec.wo) <= 0)
			return 0.0f;

		// diffuse part
		float diffPdf = INV_PI * Frame::cosTheta(bRec.wo);

		// specular part
		Vector3f bestDir = reflect(bRec.wi);
		float cos_alpha = std::max(0.0f, bestDir.dot(bRec.wo));
		float specPdf = 0.5f * (m_exp + 1.0f) * std::pow(cos_alpha, m_exp) * INV_PI;

		// linear mix
		return m_diffSamplingWeight * diffPdf + m_specSamplingWeight * specPdf;
	}

	/// Draw a a sample from the BRDF model

	Color3f sample(BSDFQueryRecord &bRec, const Point2f &sample_) const {
		Point2f sample(sample_);
		if (Frame::cosTheta(bRec.wi) <= 0)
			return Color3f(0.0f);

		bRec.measure = ESolidAngle;

		// 1. Select diffuse or specular
		bool useSpecular = true;
		if (sample.x() <= m_specSamplingWeight) {
			sample.x() /= m_specSamplingWeight;
		} else {
			sample.x() = (sample.x() - m_specSamplingWeight) / m_diffSamplingWeight;
			useSpecular = false;
		}

		if (useSpecular) {
			// this is a tricky one
			// See http://mathinfo.univ-reims.fr/IMG/pdf/Using_the_modified_Phong_reflectance_model_for_Physically_based_rendering_-_Lafortune.pdf

			float sinTheta = std::sqrt(1.0f - std::pow(sample.y(), 2.0f/(m_exp + 1.0f)));
			float cosTheta = std::pow(sample.y(), 1.0f/(m_exp + 1.0f));
			float phi = 2.0f * M_PI * sample.x();
			float cosPhi, sinPhi;
			sincosf(phi, &sinPhi, &cosPhi);
			// direction from the lobe axis (i.e. reflection of wi) in lobe coordinate
			Vector3f lobeAxis = reflect(bRec.wi);
			Vector3f lobeWo(
					sinTheta * std::cos(phi),
					sinTheta * std::sin(phi),
					cosTheta
			);
			bRec.wo = Frame(lobeAxis).toWorld(lobeWo);

			// check that we are on the hemisphere
			if(Frame::cosTheta(bRec.wo) <= 0.0f)
				return Color3f(0.0f);
		} else {
			/* Warp a uniformly distributed sample on [0,1]^2
				to a direction on a cosine-weighted hemisphere */
			bRec.wo = squareToCosineHemisphere(sample);
		}

		/* Relative index of refraction: no change */
		bRec.eta = 1.0f;

		// the importance-weighted sample is given by
		//      f / pdf * cos(wo)
		Color3f f_eval = eval(bRec);
		float pdf_eval = pdf(bRec);
		if(pdf_eval <= 0.0f) return Color3f(0.0f); // let's not explode here

		return f_eval / pdf_eval * Frame::cosTheta(bRec.wo);
	}

	/// Return a human-readable summary

	QString toString() const {
		return QString(
				"Phong[\n"
				"  Kd = %1\n"
				"  Ks = %2\n"
				"  n  = %3\n"
				"]").arg(m_Kd.toString()).arg(m_Ks.toString()).arg(m_exp);
	}

	Color3f getColor() const {
		return m_Kd;
	}

	EClassType getClassType() const {
		return EBSDF;
	}
private:
	float m_diffSamplingWeight, m_specSamplingWeight;
	Color3f m_Kd, m_Ks;
	float m_exp;
};

NORI_REGISTER_CLASS(Phong, "phong");
NORI_NAMESPACE_END
