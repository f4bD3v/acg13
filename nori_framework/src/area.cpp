/*
        Mostly unimplemented simple area luminaire
*/

#include <nori/luminaire.h>
#include <nori/mesh.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Simple area luminaire with uniform emittance
 */
class AreaLuminaire : public Luminaire {
public:
	AreaLuminaire(const PropertyList &propList) : m_mesh(NULL) {
		/* Emitted radiance */
		m_radiance = propList.getColor("radiance");
	}

	Color3f sample(LuminaireQueryRecord &, 
			const Point2f &) const {
		throw NoriException("AreaLuminaire::sample() is not implemented!");
	}

	float pdf(const LuminaireQueryRecord &) const {
		throw NoriException("AreaLuminaire::pdf() is not implemented!");
	}

	Color3f eval(const LuminaireQueryRecord &lRec) const {
                // Are we on the good side?
		return -lRec.n.dot(lRec.d) > 0 ? m_radiance : Color3f(0.0f);
	}
	
	bool isEnvironmentLuminaire() const {
		return false;
	}

	void setParent(NoriObject *object) {
		if (object->getClassType() != EMesh)
			throw NoriException("AreaLuminaire: attached to a non-mesh object!");
		m_mesh = static_cast<Mesh *>(object);
	}
        
        const NoriObject *getParent() const { return m_mesh; }
        
        Color3f getColor() const { return m_radiance; }

	QString toString() const {
		return QString("AreaLuminaire[radiance=%1]").arg(m_radiance.toString());
	}
private:
	Color3f m_radiance;
	Mesh *m_mesh;
};

NORI_REGISTER_CLASS(AreaLuminaire, "area");
NORI_NAMESPACE_END
