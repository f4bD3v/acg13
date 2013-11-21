/*
    This file is part of XYZ TODO have something useful here

    Copyright (c) 2013 LGG

    XXX license things to add here
*/

#include <nori/integrator.h>
#include <nori/sampler.h>
#include <nori/scene.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Depth mapping: a simple rendering technique that 
 * displays the depth of an object.
 */
class Depth : public Integrator {
public:
	Depth(const PropertyList &propList) {
                /* Depth near and far plane distance */
                m_near = propList.getFloat("near", 1e-4);
                m_far = propList.getFloat("far", 1e2);
                /* Min intensity */
                m_Ka = propList.getFloat("ambiant", 0.1);
                m_gamma = propList.getFloat("gamma", 5.0);
	}

	Color3f Li(const Scene *scene, Sampler *, const Ray3f &ray) const {
                
		/* Find the surface that is visible in the requested direction */
		Intersection its;
		if (!scene->rayIntersect(ray, its))
			return Color3f(0.0f);
                
                // Computation of the distance
                float depth = its.t;

		/* Perform an occlusion test and return one or zero depending on the result */
                float Kd = std::max(0.0f, 1.0f - (depth - m_near) / (m_far - m_near));
		return Color3f(m_Ka + (1.0f - m_Ka) * std::pow(Kd, m_gamma));
	}

	QString toString() const {
		return QString("Depth[near=%1, far=%2, Ka=%3]").arg(m_near).arg(m_far).arg(m_Ka);
	}
private:
	float m_near, m_far, m_Ka, m_gamma; // m_length;
};

NORI_REGISTER_CLASS(Depth, "depth");
NORI_NAMESPACE_END
