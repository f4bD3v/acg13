/*******************************************************************************
 *  meshentry.h
 *******************************************************************************
 *  Copyright (c) 2013 Alexandre Kaspar <alexandre.kaspar@a3.epfl.ch>
 *  For Advanced Computer Graphics, at the LGG / EPFL
 * 
 *        DO NOT REDISTRIBUTE
 ***********************************************/

#ifndef _MESHENTRY_H
#define	_MESHENTRY_H

#include <nori/designer.h>
#include <nori/mesh.h>
#include <nori/transform.h>
#include <nori/vector.h>
#include <QGLBuffer>
#include <QListWidgetItem>
#include <QString>

/**
 * \brief Mesh entry within the designer tool
 */
class MeshEntry {
public:

    MeshEntry(Designer *p, const Mesh *mesh, QListWidgetItem *item_);

    virtual ~MeshEntry();
    
    const QString &name() const;
    bool hasOriginalTransform() const;
    void applyTransform();
    
    void translate(const Vector3f &d);
    void rotate(float degrees, const Vector3f &axis);
    void scale(const Vector3f &scale);
    
    void resetTransform();
    void remove();
    inline bool isValid() {
        return valid;
    }

public:
    // data
    GLuint dataCount, indexCount;
    GLfloat *vertexBuffer;
    GLfloat *normalBuffer;
    GLuint *indexBuffer;
    // vbo
    bool initVBO, useVBO;
    QGLBuffer vertexVBO, normalVBO, indexVBO;

    // scene properties (transform and color)
    Color3f color;
    bool light, visible, selected;
    const Transform originalTransform;
    Transform transform;
    // current transform
    // Vector3f pos, rot, scale;

private:
    // parent
    Designer *parent;
    QString meshName;
    bool valid;
    
public:
    // corresponding item
    QListWidgetItem *item;
};

#endif /* _MESHENTRY_H */