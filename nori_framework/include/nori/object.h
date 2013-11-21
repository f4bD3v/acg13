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

#if !defined(__OBJECT_H)
#define __OBJECT_H

#include <QDebug>
#include <QString>
#include <nori/proplist.h>
#include <boost/function.hpp>

NORI_NAMESPACE_BEGIN

/**
 * \brief Base class of all objects
 *
 * A Nori object represents an instance that is part of
 * a scene description, e.g. a scattering model or luminaire.
 */
class NoriObject {
public:
	enum EClassType {
		EScene = 0,
		EMesh,
		EBSDF,
		EPhaseFunction,
		ELuminaire,
		EMedium,
		ECamera,
		EIntegrator,
		ESampler,
		ETest,
		EReconstructionFilter,
                EEvaluator,
		EClassTypeCount
	};

	/// Virtual destructor
	virtual ~NoriObject() { }

	/**
	 * \brief Return the type of object (i.e. Mesh/BSDF/etc.) 
	 * provided by this instance
	 * */
	virtual EClassType getClassType() const = 0;

	/**
	 * \brief Add a child object to the current instance
	 *
	 * The default implementation does not support children and
	 * simply throws an exception
	 */
	virtual void addChild(NoriObject *child);

	/**
	 * \brief Set the parent object
	 *
	 * Subclasses may choose to override this method to be
	 * notified when they are added to a parent object. The
	 * default implementation does nothing.
	 */
	virtual void setParent(NoriObject *parent);
        
        /**
         * \brief Return a meaningful parent
         */
        virtual const NoriObject *getParent() const { return NULL; }

	/**
	 * \brief Perform some action associated with the object
	 *
	 * The default implementation throws an exception. Certain objects
	 * may choose to override it, e.g. to implement initialization, 
	 * testing, or rendering functionality.
	 *
	 * This function is called by the XML parser once it has
	 * constructed an object and added all of its children
	 * using \ref addChild().
	 */
	virtual void activate();

	/// Return a brief string summary of the instance (for debugging purposes)
	virtual QString toString() const = 0;
	
	/// Turn a class type into a human-readable string
	inline static QString classTypeName(EClassType type) {
		switch (type) {
			case EScene:      return "scene";
			case EMesh:       return "mesh";
			case EBSDF:       return "bsdf";
			case ELuminaire:  return "luminaire";
			case ECamera:     return "camera";
			case EIntegrator: return "integrator";
			case ESampler:    return "sampler";
			case ETest:       return "test";
                        case EEvaluator:  return "evaluator";         
			default:          return "<unknown>";
		}
	}
};

/**
 * \brief Factory for Nori objects
 *
 * This utility class is part of a mini-RTTI framework and can 
 * instantiate arbitrary Nori objects by their name.
 */
class NoriObjectFactory {
public:
	typedef boost::function<NoriObject *(const PropertyList &)> Constructor;
        typedef unsigned int Version;

	/**
	 * \brief Register an object constructor with the object factory
	 *
	 * This function is called by the macro \ref NORI_REGISTER_CLASS
	 *
	 * \param name
	 *     An internal name that is associated with this class. This is the
	 *     'type' field found in the scene description XML files
	 *
	 * \param constr
	 *     A Boost function pointer to an anonymous function that is
	 *     able to call the constructor of the class.
	 */
	static void registerClass(const QString &name, const Constructor &constr, Version version);
        
        /**
         * \brief Select the current version to use when different versions are available
         * 
         * \param v
         *      The version to enforce
         */
        static void setVersion(Version v) {
                m_currentVersion = v;
        }
        
        /**
         * \brief Return the currently enforced version
         */
        static Version version() {
                return m_currentVersion;
        }
        
        /**
         * \brief Set the base directory
         */
        static void setBasedir(const QString& dirname){
                m_basedir = dirname;
        }
        
        /**
         * \brief Return the current base directory
         */
        static QString basedir(){
                return m_basedir;
        }

	/**
	 * \brief Construct an instance from the class of the given name
	 *
	 * \param name
	 *     An internal name that is associated with this class. This is the
	 *     'type' field found in the scene description XML files
	 *
	 * \param propList
	 *     A list of properties that will be passed to the constructor
	 *     of the class.
	 */
	inline static NoriObject *createInstance(const QString &name,
			const PropertyList &propList) {
        qDebug() << "constructing " << name;
                const Version ver = m_currentVersion;
		if (m_constructors->find(name) == m_constructors->end())
			throw NoriException(QString("A constructor for class '%1' "
				"could not be found!").arg(name));
                // now we find the good version to use
                std::map<Version, Constructor> &versionMap = (*m_constructors)[name];
                // if there is only one, use it without looking at the version number
                if(versionMap.size() == 1) return versionMap.begin()->second(propList);
                // else we enforce that version number
                if(versionMap.find(ver) == versionMap.end())
                        throw NoriException(QString("Version %1 of constructor"
                                " for class '%2' could not be found!").arg(ver).arg(name));
                // we can use the enforced version
                return versionMap[ver](propList);
	}
private:
	static std::map<QString, std::map<Version, Constructor> > *m_constructors;
        static Version m_currentVersion;
        static QString m_basedir;
};

/**
 * \brief Return the absolute file which uses the current base directory
 */
QString absFileName(const QString& fname);

/// Macro for registering an object constructor with the \ref NoriObjectFactory
#define NORI_REGISTER_CLASS_IMPL(cls, cls0, name, version) \
	cls *cls0 ##_create(const PropertyList &list) { \
		return new cls(list); \
	} \
	static struct cls0 ##_{ \
		cls0 ##_() { \
			NoriObjectFactory::registerClass(name, cls0 ##_create, version); \
		} \
	} cls0 ##__;
// for macro expansion
#define NORI_REGISTER_CLASS(cls, name) NORI_REGISTER_CLASS_IMPL(cls, cls, name, 0)
#define NORI_REGISTER_CLASS_VERSION(cls, cls0, name, ver) NORI_REGISTER_CLASS_IMPL(cls, cls0, name, ver)
NORI_NAMESPACE_END

#endif /* __OBJECT_H */
