/*******************************************************************************
 *  meshentry.cpp
 *******************************************************************************
 *  Copyright (c) 2013 Alexandre Kaspar <alexandre.kaspar@a3.epfl.ch>
 *  For Advanced Computer Graphics, at the LGG / EPFL
 * 
 *        DO NOT REDISTRIBUTE
 ***********************************************/

#include <nori/designer/meshentry.h>
#include <nori/mesh.h>
#include <Eigen/Geometry>
#include <iostream>
#include <QtOpenGL>

using namespace nori;

Vector3f crossProduct(const Vector3f &a, const Vector3f &b) {
    return Vector3f(
            a.coeff(1) * b.coeff(2) - b.coeff(1) * a.coeff(2),
            a.coeff(2) * b.coeff(0) - a.coeff(0) * b.coeff(2),
            a.coeff(0) * b.coeff(1) - a.coeff(1) * b.coeff(0)
            );
}

MeshEntry::MeshEntry(Designer *p, const Mesh *mesh, QListWidgetItem *item_) :
vertexBuffer(NULL), normalBuffer(NULL), indexBuffer(NULL),
initVBO(false), useVBO(false),
color(mesh->getMainColor()), light(mesh->isLuminaire()), visible(true), selected(false),
originalTransform(mesh->getOriginalTransform()), transform(mesh->getOriginalTransform()),
// pos(0.0f), rot(0.0f), scale(1.0f),
parent(p), meshName(mesh->getName()), valid(true), item(item_) {
    dataCount = mesh->getVertexCount();
    indexCount = mesh->getTriangleCount() * 3;
    // we allocate the space
    vertexBuffer = new GLfloat[dataCount * 3];
    normalBuffer = new GLfloat[dataCount * 3];
    indexBuffer = new GLuint[indexCount];
    std::cout << "\n\nNew mesh entry:\n";
    // TODO do that with memory functions which can go much faster for contiguous memory!
    for (GLuint v = 0, i = 0; v < dataCount; ++v, i += 3) {
        const Point3f p = transform.inverse() * mesh->getVertexPositions()[v];
        std::cout << "p#" << v << ": " << p.toString().toStdString() << "\n";
        vertexBuffer[i + 0] = p(0);
        vertexBuffer[i + 1] = p(1);
        vertexBuffer[i + 2] = p(2);

    }
    // index
    for (GLuint i = 0; i < indexCount; ++i) {
        // i => linear access to indices
        // idx(i) => linear access to vertices (full)
        // idx(i) * 3 => linear access to vertex::x of [x0, y0, z0, x1, y1, z1 ... xk, yk, zk]
        indexBuffer[i] = mesh->getIndices()[i] * 3;
    }
    // normals
    if (mesh->getVertexNormals() != NULL) {
        for (GLuint v = 0, i = 0; v < dataCount; ++v, i += 3) {
            const Normal3f n = (transform.inverse() * mesh->getVertexNormals()[v]).normalized();
            normalBuffer[i + 0] = n(0);
            normalBuffer[i + 1] = n(1);
            normalBuffer[i + 2] = n(2);
        }
    } else {
        // we interpolate them
        Normal3f *vn = new Normal3f[dataCount];
        GLuint *c = new GLuint[dataCount];
        for (GLuint i = 0; i < indexCount; i += 3) {
            GLuint i0 = mesh->getIndices()[i],
                    i1 = mesh->getIndices()[i + 1],
                    i2 = mesh->getIndices()[i + 2];
            const Point3f &p0 = mesh->getVertexPositions()[i0];
            const Point3f &p1 = mesh->getVertexPositions()[i1];
            const Point3f &p2 = mesh->getVertexPositions()[i2];
            // => 3 normals
            Normal3f n = crossProduct(Vector3f(p1 - p0), (Vector3f(p2 - p0)));
            vn[i0] += n;
            c[i0] += 1;
            vn[i1] += n;
            c[i1] += 1;
            vn[i2] += n;
            c[i2] += 1;
        }
        for (GLuint i = 0, j = 0; i < dataCount; ++i, j += 3) {
            Normal3f &n = vn[i];
            normalBuffer[j + 0] = n(0) / float(c[i]);
            normalBuffer[j + 1] = n(1) / float(c[i]);
            normalBuffer[j + 2] = n(2) / float(c[i]);
        }
        // release temporary arrays
        delete[] vn;
        delete[] c;
    }
    // miscellaneous
    // - color correction
    float cMax = std::max(color.r(), std::max(color.g(), color.b()));
    if (cMax > 1.0f) color = color / cMax;
}

MeshEntry::~MeshEntry() {
    if (vertexBuffer) delete[] vertexBuffer;
    if (normalBuffer) delete[] normalBuffer;
    if (indexBuffer) delete[] indexBuffer;
    if (initVBO) {
        vertexVBO.destroy();
        normalVBO.destroy();
        indexVBO.destroy();
    }
}

const QString &MeshEntry::name() const {
    return meshName;
}

bool MeshEntry::hasOriginalTransform() const {
    return originalTransform.getMatrix() == transform.getMatrix(); // pos != Vector3f(0.0f) || rot != Vector3f(0.0f) || scale != Vector3f(1.0f);
}

void rot3d(Transform &transform, float degrees, float x, float y, float z) {
    float angle = degrees * M_PI / 180.0f;
    transform = Transform((Eigen::AngleAxis<float>(angle, Vector3f(x, y, z)) * Eigen::Affine3f(transform.getMatrix())).matrix());
}

void MeshEntry::applyTransform() {
    /*
    // translate
    transform = Transform( (Eigen::Translation<float, 3>(pos(0), pos(1), pos(2)) * Eigen::Affine3f(transform.getMatrix())).matrix() );
    // scale
    transform = Transform( (Eigen::DiagonalMatrix<float, 3>(scale) * Eigen::Affine3f(transform.getMatrix())).matrix() );
    // rotate
    rot3d(transform, rot(0), 1.0f, 0.0f, 0.0f);
    rot3d(transform, rot(1), 0.0f, 1.0f, 0.0f);
    rot3d(transform, rot(2), 0.0f, 0.0f, 1.0f);
    
    // reset partial transform
    pos = Vector3f(0.0f);
    scale = Vector3f(1.0f);
    rot = Vector3f(0.0f); */
}

inline Eigen::Affine3f affinv(const Transform &transform) {
    return Eigen::Affine3f(transform.getInverseMatrix());
}

inline Eigen::Affine3f aff(const Transform &transform) {
    return Eigen::Affine3f(transform.getMatrix());
}

void MeshEntry::translate(const Vector3f& d) {
    transform = Transform((Eigen::Translation<float, 3>(d) * aff(transform)).matrix());
}

// /!\ For rotate + scale, we apply the transformation before the current one so that it happened at origin!

void MeshEntry::rotate(float degrees, const Vector3f& axis) {
    float angle = degrees * M_PI / 180.0f;
    transform = Transform((aff(transform) * Eigen::AngleAxis<float>(angle, axis)).matrix());
}

void MeshEntry::scale(const Vector3f &scale) {
    transform = Transform((aff(transform) * Eigen::DiagonalMatrix<float, 3>(scale)).matrix());
}

void MeshEntry::resetTransform() {
    transform = originalTransform;
}

void MeshEntry::remove() {
    // delegate the real removal to the scene viewer (since in another thread!)
    valid = false;
}