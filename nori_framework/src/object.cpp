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

#include <nori/object.h>
#include <QDir>

NORI_NAMESPACE_BEGIN

void NoriObject::addChild(NoriObject *) {
	throw NoriException(QString("NoriObject::addChild() is not "
		"implemented for objects of type '%1'!").arg(
			classTypeName(getClassType())));
}

void NoriObject::activate() { /* Do nothing */ }
void NoriObject::setParent(NoriObject *) { /* Do nothing */ }

typedef std::map<NoriObjectFactory::Version, NoriObjectFactory::Constructor> ConstructorVersions;
std::map<QString, ConstructorVersions> *NoriObjectFactory::m_constructors = NULL;
NoriObjectFactory::Version NoriObjectFactory::m_currentVersion = 0; // default version
QString NoriObjectFactory::m_basedir("."); // default base directory

void NoriObjectFactory::registerClass(const QString &name, const Constructor &constr, Version ver) {
	if (!m_constructors)
		m_constructors = new std::map<QString, ConstructorVersions>();
	(*m_constructors)[name][ver] = constr;
}

QString absFileName(const QString& fname){
        return QString("%1%2%3").arg(NoriObjectFactory::basedir()).arg(QDir::separator()).arg(fname);
}

NORI_NAMESPACE_END
