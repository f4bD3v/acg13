/*******************************************************************************
 *  viewer.cpp
 *******************************************************************************
 *  Copyright (c) 2013 Alexandre Kaspar <alexandre.kaspar@a3.epfl.ch>
 *  For Advanced Computer Graphics, at the LGG / EPFL
 * 
 *        DO NOT REDISTRIBUTE
 ***********************************************/



#ifndef _GLSCENEVIEWER_H
#define _GLSCENEVIEWER_H

#include <nori/transform.h>
#include <nori/vector.h>
#include <QGLShader>
#include <QGLWidget>
#include <QAction>
#include <QListWidgetItem>
#include <QMenu>
#include <QScopedPointer>

class Designer;
class MeshEntry;

using namespace nori;

/**
 * GL Scene Viewer as a widget
 */
class GLSceneViewer : public QGLWidget {
    Q_OBJECT
public:
    
    enum MouseMode {
        CameraMode,
        GrabMode,
        RotateMode,
        ScaleMode
    };

    GLSceneViewer(QWidget *parent);

protected:

    /// The shader initialization
    
    void initializeGL();

    void resizeGL(int w, int h);

    /// Time!
    GLuint k;

    /// The main drawing implementation
    
    void paintGL();

    // Draw sub parts
    
    bool initVBO(MeshEntry *mesh);
    void transformMesh(MeshEntry *mesh);
    void displayVBO(MeshEntry *mesh);
    void displayMesh(MeshEntry *mesh, bool debug = false);

protected:

    /// Event implementations
    
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void showContextMenu(const QPoint &pos);
    void contextMenuEvent(QContextMenuEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

protected:
    
    void rotate(float rot, const Vector3f& axis);
    void translate(const Vector3f& v);
    // orientation from camera
    Vector3f currentTarget() const;
    Vector3f currentUp() const;
    Vector3f currentRight() const;

public:
    
    void resetCamera();
    Transform initMeshTransform() const;
    Transform cameraTransform() const;
    inline void setInitialTransform(const Transform &t) {
        initCamTransform = t; 
   }
    
    // Mesh stuff
    void addMesh(MeshEntry *mesh);
    void clear();
    
public slots:
    void selectionChanged( QListWidgetItem *cur, QListWidgetItem *prev);

private:
    Designer *d;
    QVector<MeshEntry *> meshes;
    int w, h;
    // shader program
    QGLShaderProgram m_program;
    int paramLocation;
    // scene transformations
    Transform initCamTransform, camTransform;
    // dragging
    bool dragging;
    int lastX, lastY;
    // context menu and actions
    QScopedPointer<QMenu> menu;
    QScopedPointer<QAction> vboAction, resetCameraAction, timedLightAction;
    bool usingVBO, varyingLight;
    // mouse interaction
    MouseMode mouseMode;
};

#endif /* _GLSCENEVIEWER_H */
