/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2012 by Wenzel Jakob and Steve Marschner.

    Nori is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Nori is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#if !defined(__LUMINAIRE_H)
#define __LUMINAIRE_H

#include <nori/object.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Data record for conveniently querying and sampling the
 * direct illumination technique implemented by a luminaire
 */
struct LuminaireQueryRecord {
	/// Pointer to the sampled luminaire
	const Luminaire *luminaire;
	/// Reference position
	Point3f ref;
	/// Sampled position on the light source
	Point3f p;
	/// Associated surface normal
	Normal3f n;
	/// Solid angle density wrt. 'ref'
	float pdf;
	/// Direction vector from 'ref' to 'p'
	Vector3f d;
	/// Distance between 'ref' and 'p'
	float dist;

	/// Create an unitialized query record
	inline LuminaireQueryRecord() : luminaire(NULL) { }

	/// Create a new query record that can be used to sample a luminaire
	inline LuminaireQueryRecord(const Point3f &ref) : ref(ref) { }

	/**
	 * \brief Create a query record that can be used to query the 
	 * sampling density after having intersected an area luminaire
	 */
	inline LuminaireQueryRecord(const Luminaire *luminaire, 
			const Point3f &ref, const Point3f &p,
			const Normal3f &n) : luminaire(luminaire), ref(ref), p(p), n(n) {
		d = p - ref;
		dist = d.norm();
		d /= dist;
	}

	/**
	 * \brief Create a query record that can be used to query the 
	 * sampling density after having intersected an environment luminaire
	 */
	inline LuminaireQueryRecord(const Luminaire *luminaire, const Ray3f &ray) :
		luminaire(luminaire), ref(ray.o), p(ray(1)), n(-ray.d), d(ray.d), 
		dist(std::numeric_limits<float>::infinity()) {
	}


	/// Return a human-readable string summary
	inline QString toString() const;
};

/**
 * \brief Superclass of all bidirectional scattering distribution functions
 */
class Luminaire : public NoriObject {
public:
	/**
	 * \brief Direct illumination sampling: given a reference point in the 
	 * scene, sample an luminaire position that contributes towards it
	 * or fail.
	 *
	 * Given an arbitrary reference point in the scene, this method 
	 * samples a position on the luminaire that has a nonzero contribution 
	 * towards that point. Note that this does not yet account for visibility.
	 * 
	 * Ideally, the implementation should importance sample the product of
	 * the emission profile and the geometry term between the reference point 
	 * and the position on the luminaire.
	 *
	 * \param lRec
	 *    A lumaire query record that specifies the reference point.
	 *    After the function terminates, it will be populated with the 
	 *    position sample and related information
	 *
	 * \param sample
	 *    A uniformly distributed 2D vector
	 *
	 * \return
	 *    An importance weight associated with the sample. Includes
	 *    any geometric terms between the luminaire and the reference point.
	 */
	virtual Color3f sample(LuminaireQueryRecord &lRec, 
			const Point2f &sample) const = 0;

	/**
	 * \brief Compute the sampling density of the direct illumination technique
	 * implemented by \ref sample() with respect to the solid angle measure
	 */
	virtual float pdf(const LuminaireQueryRecord &lRec) const = 0;

	/// Evaluate the emitted radiance
	virtual Color3f eval(const LuminaireQueryRecord &lRec) const = 0;

	/// Is this an environment luminaire?
	virtual bool isEnvironmentLuminaire() const = 0;
        
        virtual Color3f getColor() const { return Color3f(1.0f); }

	/**
	 * \brief Return the type of object (i.e. Mesh/Luminaire/etc.) 
	 * provided by this instance
	 * */
	EClassType getClassType() const { return ELuminaire; }
};

inline QString LuminaireQueryRecord::toString() const {
	return QString(
		"LuminaireQueryRecord[\n"
		"  luminaire = \"%1\",\n"
		"  ref = %2,\n"
		"  p = %3,\n"
		"  n = %4,\n"
		"  pdf = %5,\n"
		"  d = %6,\n"
		"  dist = %7\n"
		"]")
	.arg(indent(luminaire->toString()))
	.arg(ref.toString())
	.arg(p.toString())
	.arg(n.toString())
	.arg(pdf)
	.arg(d.toString())
	.arg(dist);
}

NORI_NAMESPACE_END

#endif /* __LUMINAIRE_H */
