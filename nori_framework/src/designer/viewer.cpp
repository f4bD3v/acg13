/*******************************************************************************
 *  viewer.cpp
 *******************************************************************************
 *  Copyright (c) 2013 Alexandre Kaspar <alexandre.kaspar@a3.epfl.ch>
 *  For Advanced Computer Graphics, at the LGG / EPFL
 * 
 *        DO NOT REDISTRIBUTE
 ***********************************************/

#include <nori/designer/viewer.h>
#include <nori/designer/meshentry.h>
#include <nori/common.h>
#include <Eigen/Geometry>
#ifdef __APPLE__
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif
#include <QTimer>
#include <iostream>

using namespace nori;

/// Helpers

inline void buildPerspectiveFov(float *matrix, float fov, float width, float height, float zNear, float zFar) {
    float hfov = fov * M_PI / 360.0; // radians!
    float h = std::cos(hfov) / std::sin(hfov);
    float w = h * height / width;
    matrix[0] = w;
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;
    matrix[4] = 0.0f;
    matrix[5] = h;
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;
    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = -(zFar + zNear) / (zFar - zNear);
    matrix[11] = -1.0f;
    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = -(2.0f * zFar * zNear) / (zFar - zNear);
    matrix[15] = 0.0f;
}

void gluPerspectiveFov(float fov, float width, float height, float zNear, float zFar) {
    float m[16];
    buildPerspectiveFov(&m[0], fov, width, height, zNear, zFar);
    glLoadMatrixf(&m[0]);
}

//
// ##### Constructor ###########################################################
//

/// Add a scene action
#define SCENE_ACTION(what, str) \
        act = new QAction(str, this); \
        what##Action.reset(act); \
        menu->addAction(act)

GLSceneViewer::GLSceneViewer(QWidget *parent)
: QGLWidget(parent), d(static_cast<Designer *> (parent)), usingVBO(false), varyingLight(false), mouseMode(CameraMode) {
    // create context menu
    setContextMenuPolicy(Qt::DefaultContextMenu);
    menu.reset(new QMenu());
    QAction *act;
    SCENE_ACTION(resetCamera, "Reset Camera");
    SCENE_ACTION(vbo, "Use VBO");
    SCENE_ACTION(timedLight, "Time varying lights");

    // focus for key events
    setFocusPolicy(Qt::StrongFocus);
}

//
// ##### Shader program ########################################################
//

void GLSceneViewer::initializeGL() {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glDisable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);

    // display shader

    if (!m_program.addShaderFromSourceCode(QGLShader::Vertex,
            "uniform float param;\n"
            "varying vec3 v;\n"
            "varying vec3 n;\n"
            "varying vec4 color;\n"
            "void main() {\n"
            "	v = -normalize(vec3(gl_ModelViewMatrix * gl_Vertex));\n"
            "       n = normalize(gl_NormalMatrix * gl_Normal);\n"
            "       color = gl_Color;\n"
            "       gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
            "}\n"))
        throw NoriException("Could not compile vertex shader!");

    if (!m_program.addShaderFromSourceCode(QGLShader::Fragment,
            "#version 120\n"
            "uniform float param;\n" // param = light variation with time
            "varying vec3 v;\n"
            "varying vec3 n;\n"
            "varying vec4 color;\n"
            "\n"
            "#define pi 3.141592653589793238462643383279\n"
            "\n"
            "float wave(float a, float b, float d, float x){\n"
            "       return a + (b - a) * 0.5 * (1.0 + cos(2.0 * pi * x / d));\n"
            "}\n"
            "void main() {\n"
            "       float L = 1.0;\n"
            "       if(param > 0.0) L = wave(0.5, 1.5, 10.0, param);\n" // light changing with time
            "       gl_FragColor = vec4(color.rgb * max(dot(n, v), 0.0) * L, color.a);\n"
            "}\n"))
        throw NoriException("Could not compile fragment shader!");

    if (!m_program.link())
        throw NoriException("Could not link shader!");

    // location of the params uniform
    paramLocation = m_program.uniformLocation("param");
}

void GLSceneViewer::resizeGL(int w, int h) {
    glViewport(0, 0, this->w = w, this->h = h);
}

//
// ##### GL drawing ############################################################
//

void GLSceneViewer::paintGL() {
    ++k;
    // std::cout << "paintGL() #" << k << "\n";
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind the shader program
    m_program.bind();

    // 1 = set camera perspective and initial translation
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspectiveFov(float(d->widget.fov->value()),
            float(d->widget.viewWidth->value()), float(d->widget.viewHeight->value()),
            0.0001f, 10000.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(1.0f, 1.0f, -1.0f); // opengl has inverse z axis w.r.t. Nori
    glMultMatrixf(initCamTransform.data()); // column-major!
    glMultMatrixf(camTransform.data());

    // 2 = display meshes
    {
        bool withVBO = usingVBO;
        QVector<int> toRemove;
        for (int i = 0; i < meshes.size(); ++i) {
            // std::cout << "Displaying mesh#" << i << "\n";
            MeshEntry *m = meshes[i];
            if (!m->isValid()) {
                // we set its entry to be removed
                toRemove.push_back(i);
                continue;
            }
            if (!m->visible) continue; // we don't display it

            // shader uniform
            m_program.setUniformValue(paramLocation, m->light && varyingLight ? float(k) : 0.0f);

            // transform to apply
            glPushMatrix();
            {
                // apply transform
                transformMesh(m);

                // display mesh
                bool selected = m->selected;
                if (selected) glDisable(GL_BLEND);
                else glEnable(GL_BLEND);
                glColor4f(m->color.r(), m->color.g(), m->color.b(), selected ? 1.0f : 0.5f);
                if (withVBO && !m->initVBO) m->useVBO = initVBO(m);
                if (withVBO && m->useVBO) displayVBO(m);
                else displayMesh(m, selected);
            }
            glPopMatrix();
        }
        if (toRemove.size() > 0) {
            for (int i = 0, n = toRemove.size(); i < n; ++i) {
                MeshEntry *m = meshes[i];
                meshes.remove(toRemove[i] - i); // - i => rows shifted left for each one we removed!
                // we free it ... later
                delete m;
            }

        }
    }
    // done displaying the meshes
    m_program.release();

    // 3 = display camera frame
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 1, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    {
        glLoadIdentity();
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        float dw(w - d->widget.viewWidth->value()), dh(h - d->widget.viewHeight->value());
        dw /= w;
        dh /= h;
        float dx = dw / 2.0f, dy = dh / 2.0f;
        glBegin(GL_TRIANGLE_STRIP);
        glColor4f(1.0f, 1.0f, 1.0f, 0.3f);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(dx, dy);
        glVertex2f(1.0f, 0.0f);
        glVertex2f(1.0f - dx, dy);
        glVertex2f(1.0f, 1.0f);
        glVertex2f(1.0f - dx, 1.0f - dy);
        glVertex2f(0.0f, 1.0f);
        glVertex2f(dx, 1.0f - dy);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(dx, dy);
        glEnd();
        glBegin(GL_LINE_LOOP);
        float dm = 5.0f / std::min(w, h);
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
        glVertex2f(dx - dm, dy - dm);
        glVertex2f(1.0f - dx + dm, dy - dm);
        glVertex2f(1.0f - dx + dm, 1.0f - dy + dm);
        glVertex2f(dx - dm, 1.0f - dy + dm);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glVertex2f(dx, dy);
        glVertex2f(1.0f - dx, dy);
        glVertex2f(1.0f - dx, 1.0f - dy);
        glVertex2f(dx, 1.0f - dy);
        glEnd();

        // text display
        glPushMatrix();
        {
            // Mouse Mode Text
            int x = 20, y = 30;
            QString text;
            switch (mouseMode) {
                case CameraMode: text = "Camera Mode";
                    break;
                case GrabMode: text = "Grab Mode";
                    break;
                case RotateMode: text = "Rotate Mode";
                    break;
                case ScaleMode: text = "Scale Mode";
                    break;
                default: throw NoriException("Unknown Mouse Mode!");
            }
            QFont f = font();
            f.setBold(true);
            glColor3f(0.0f, 0.0f, 0.0f);
            this->renderText(x - 1, y - 1, text, f);
            this->renderText(x + 1, y + 1, text, f);
            glColor3f(1.0f, 1.0f, 1.0f);
            this->renderText(x, y, text, f);
        }
        glPopMatrix();

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }
}

//
//
//

void GLSceneViewer::transformMesh(MeshEntry* m) {
    // current set transform
    glMultMatrixf(m->transform.data()); // column-major!
    /*
    // translation
    glTranslatef(m->pos.coeff(0), m->pos.coeff(1), m->pos.coeff(2));
    // scaling
    glScalef(m->scale.coeff(0), m->scale.coeff(1), m->scale.coeff(2));
    // rotation
    glRotatef(m->rot.coeff(0), 1.0f, 0.0f, 0.0f);
    glRotatef(m->rot.coeff(1), 0.0f, 1.0f, 0.0f);
    glRotatef(m->rot.coeff(2), 0.0f, 0.0f, 1.0f);
     */
}

//
// ##### Drawing parts #########################################################
//

#define SAFE_TRY if(!
#define OR_EXIT ) return false

bool GLSceneViewer::initVBO(MeshEntry *mesh) {
    // std::cout << "Init VBO: " << mesh->objectName().toStdString() << "\n";
    mesh->initVBO = true;
    // vertex VBO
    mesh->vertexVBO.setUsagePattern(QGLBuffer::StaticDraw);
    SAFE_TRY mesh->vertexVBO.create() OR_EXIT;
    SAFE_TRY mesh->vertexVBO.bind() OR_EXIT;
    mesh->vertexVBO.allocate(mesh->vertexBuffer, mesh->dataCount * sizeof (GLfloat));
    mesh->vertexVBO.release();
    // normal VBO
    mesh->normalVBO.setUsagePattern(QGLBuffer::StaticDraw);
    SAFE_TRY mesh->normalVBO.create() OR_EXIT;
    SAFE_TRY mesh->normalVBO.bind() OR_EXIT;
    mesh->normalVBO.allocate(mesh->normalBuffer, mesh->dataCount * sizeof (GLfloat));
    mesh->normalVBO.release();
    // normal VBO
    mesh->indexVBO.setUsagePattern(QGLBuffer::StaticDraw);
    SAFE_TRY mesh->indexVBO.create() OR_EXIT;
    SAFE_TRY mesh->indexVBO.bind() OR_EXIT;
    mesh->indexVBO.allocate(mesh->indexBuffer, mesh->indexCount * sizeof (GLuint));
    mesh->indexVBO.release();
    /*
    glGenBuffers( 2, &mesh->vbo[0] );
    // Buffer d'informations de vertex
    glBindBufferARB(GL_ARRAY_BUFFER, mesh->vbo[0]);
    glBufferDataARB(GL_ARRAY_BUFFER, sizeof(GLfloat) * mesh->dataCount * 6, mesh->dataBuffer, GL_STATIC_DRAW);

    // Buffer d'indices
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, mesh->vbo[1]);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh->indexCount, mesh->indexBuffer, GL_STATIC_DRAW);
     */
    return true; // success!
}

void GLSceneViewer::displayVBO(MeshEntry *mesh) {
    // std::cout << "Display VBO: " << mesh->objectName().toStdString() << "\n";
    // activate the different arrays
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    // data buffers
    mesh->vertexVBO.bind();
    glVertexPointer(3, GL_FLOAT, 0, NULL);

    mesh->normalVBO.bind();
    glNormalPointer(GL_FLOAT, 0, NULL);

    // drawing, FINALLY!
    mesh->indexVBO.bind();
    glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);

    // release the buffers
    mesh->indexVBO.release();
    mesh->vertexVBO.release();
    mesh->normalVBO.release();

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void GLSceneViewer::displayMesh(MeshEntry *mesh, bool debug) {
    // std::cout << "Display Mesh: " << mesh->objectName().toStdString() << "\n";
    // the naive slow way...
    glBegin(GL_TRIANGLES);
    for (GLuint i = 0, n = mesh->indexCount; i < n; ++i) {
        GLuint idx = mesh->indexBuffer[i];
        glNormal3fv(&mesh->normalBuffer[idx]);
        glVertex3fv(&mesh->vertexBuffer[idx]);
        // debug
        if (debug && false) {
            std::cout << "v#" << i << ":" << idx << ": " << mesh->vertexBuffer[idx] << ", " << mesh->vertexBuffer[idx + 1] << ", " << mesh->vertexBuffer[idx + 2] << "\n";
        }
    }
    glEnd();
}

//
// ##### Event implementations #################################################
//

enum ScrollMode {
    ScrollZ,
    ScrollX,
    ScrollY
};

ScrollMode getScrollMode(QInputEvent *event) {
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        return ScrollX;
    } else if (event->modifiers().testFlag(Qt::ShiftModifier)) {
        return ScrollY;
    } else {
        return ScrollZ;
    }
}

void GLSceneViewer::wheelEvent(QWheelEvent *event) {
    event->accept();
    float d = event->delta() > 0 ? 1.0f : -1.0f;
    d *= 0.25f; // let's not be too rough!
    ScrollMode m = getScrollMode(event);
    translate(
            Vector3f(
            m == ScrollX ? d : 0.0f,
            m == ScrollY ? d : 0.0f,
            m == ScrollZ ? d : 0.0f
            )
            );
}

void GLSceneViewer::mousePressEvent(QMouseEvent *event) {
    event->accept();
    dragging = true;
    lastX = event->x();
    lastY = event->y();
}

void GLSceneViewer::mouseMoveEvent(QMouseEvent *event) {
    if (dragging) {
        event->accept();
        // displacement
        float dx = float(lastX - event->x()) / float(w);
        float dy = float(lastY - event->y()) / float(w);
        if (std::abs(dx) > 0.01) {
            lastX = event->x(); // consume X displacement
        } else dx = 0.0f; // reject X change
        if (std::abs(dy) > 0.01) {
            lastY = event->y(); // consume Y displacement
        } else dy = 0.0f; // reject Y change

        // no change, nothing to do...
        if (dx == 0.0f && dy == 0.0f) return;

        if (mouseMode == CameraMode) {
            if (dx != 0.0f) rotate(dx * 180.0f, Vector3f(0.0f, 1.0f, 0.0f));
            if (dy != 0.0f) rotate(dy * 180.0f, Vector3f(1.0f, 0.0f, 0.0f));
        } else {
            for (int i = 0, n = meshes.size(); i < n; ++i) {
                MeshEntry *mesh = meshes[i];
                if (!mesh->selected) continue; // only for the chosen ones!
                switch (mouseMode) {
                    case GrabMode:
                        mesh->translate((dx * currentRight() + dy * currentUp()) * 2.0f);
                        break;
                    case RotateMode:
                        if (dx != 0.0f) mesh->rotate(dx * 180.0f, Vector3f(0.0f, 1.0f, 0.0f));
                        if (dy != 0.0f) mesh->rotate(dy * 180.0f, Vector3f(1.0f, 0.0f, 0.0f));
                        break;
                    case ScaleMode:
                        if (dx == 0.0f) return; // no scaling
                    {
                        float ds = std::sqrt(dx * dx + dy * dy);
                        mesh->scale(Vector3f(1.0f + ds * (dx > 0 ? -1.0f : 1.0f)));
                    }
                        break;
                    case CameraMode:
                    default:
                        throw NoriException(QString("Invalid mode '%1'!").arg(mouseMode));
                }
            }
        }
    } else {
        event->ignore();
    }
}

void GLSceneViewer::mouseReleaseEvent(QMouseEvent *event) {
    if (dragging) {
        event->accept();
        dragging = false;
    } else {
        event->ignore();
    }
}

void GLSceneViewer::showContextMenu(const QPoint &pos) {
    // for most widgets
    QPoint globalPos = mapToGlobal(pos);
    // for QAbstractScrollArea and derived classes you would use:
    // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos); 

    menu->popup(globalPos);
}

void GLSceneViewer::contextMenuEvent(QContextMenuEvent *event) {
    // for most widgets
    QPoint globalPos = mapToGlobal(event->pos());
    // for QAbstractScrollArea and derived classes you would use:
    // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos); 
    QAction *choice = menu->exec(globalPos);
    if (choice == resetCameraAction.data()) {
        resetCamera();
    } else if (choice == vboAction.data()) {
        usingVBO = !usingVBO;
        vboAction->setText(usingVBO ? "Disable VBO" : "Use VBO");
    } else if (choice == timedLightAction.data()) {
        varyingLight = !varyingLight;
        timedLightAction->setText(varyingLight ? "Fix lights" : "Time varying lights");
    }
}

void GLSceneViewer::keyReleaseEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_G:
            mouseMode = GrabMode;
            break;
        case Qt::Key_R:
            mouseMode = RotateMode;
            break;
        case Qt::Key_S:
            mouseMode = ScaleMode;
            break;
        default:
            mouseMode = CameraMode;
            break;
    }
}

//
// ##### Simple transformations ################################################
//

void GLSceneViewer::rotate(float rot, const Vector3f& axis) {
    float angle = rot * M_PI / 180.0f;
    camTransform = Transform((Eigen::AngleAxis<float>(angle, axis.normalized()) * Eigen::Affine3f(camTransform.getMatrix())).matrix());
}

void GLSceneViewer::translate(const Vector3f& v) {
    camTransform = Transform((Eigen::Translation<float, 3>(v) * Eigen::Affine3f(camTransform.getMatrix())).matrix());
}

void GLSceneViewer::resetCamera() {
    camTransform = Transform();
}

Vector3f GLSceneViewer::currentRight() const {
    const Eigen::Matrix4f &m = camTransform.getInverseMatrix();
    return -Vector3f(m(0, 0), m(1, 0), m(2, 0));
}

Vector3f GLSceneViewer::currentUp() const {
    const Eigen::Matrix4f &m = camTransform.getInverseMatrix();
    return Vector3f(m(0, 1), m(1, 1), m(2, 1));
}

Vector3f GLSceneViewer::currentTarget() const {
    const Eigen::Matrix4f &m = camTransform.getInverseMatrix();
    return Vector3f(m(0, 2), m(1, 2), m(2, 2));
}

Transform GLSceneViewer::initMeshTransform() const {
    Transform camToWorld = camTransform.inverse();
    return Transform((Eigen::Translation<float, 3>(0.0f, 0.0f, 2.0f) * Eigen::Affine3f(camToWorld.getMatrix())).matrix());
}

Transform GLSceneViewer::cameraTransform() const {
    return Transform((
        Eigen::Affine3f(camTransform.getInverseMatrix())
        * Eigen::Affine3f(initCamTransform.getInverseMatrix())
    ).matrix());
}

void GLSceneViewer::addMesh(MeshEntry* mesh) {
    meshes.push_back(mesh);
}

void GLSceneViewer::clear() {
    // clear the content
    for (int i = meshes.size() - 1; i >= 0; --i) {
        meshes[i]->remove(); // /!\ do it in this order to ensure that the indices don't change
    }

    // reset transform to identity
    resetCamera();
}

/// /!\ This last method is dangerous as its elements are from another thread.
///     Make sure you access any cautiously as *cur / *prev may be invalid!

void GLSceneViewer::selectionChanged(QListWidgetItem* cur, QListWidgetItem* prev) {
    // std::cout << "cur=" << cur << ", prev=" << prev << "\n";
    if (prev != NULL) {
        MeshEntry *oldMesh = static_cast<MeshEntry *> (prev->data(Qt::UserRole).value<void *>());
        // std::cout << "oldMesh=" << oldMesh << "\n";
        if (oldMesh != NULL) oldMesh->selected = false;
        else {
            // our item is dead!
            // let's finish it!
            delete prev;
        }
    }
    
    if (cur != NULL) {
        MeshEntry *newMesh = static_cast<MeshEntry *> (cur->data(Qt::UserRole).value<void *>());
        // std::cout << "newMesh=" << newMesh << "\n";
        if (newMesh != NULL) newMesh->selected = true;
    }
}
