//=============================================================================
//
//   Exercise code for the lecture
//   "Advanced Computer Graphics"
//
//   Adapted from Prof. Dr. Mario Botsch, Bielefeld University
//
//   Copyright (C) 2013 LGG, epfl
//
//   DO NOT REDISTRIBUTE
//=============================================================================

//== INCLUDES =================================================================


#include "utils/Viewer_2D.h"
#include "utils/gl.h"
#include "utils/gl_text.h"
#include <sstream>


//== IMPLEMENTATION ========================================================== 


Viewer_2D::Viewer_2D(const char* _title, int _width, int _height)
: GLUT_viewer(_title, _width, _height)
{
    animate_     = false;
    time_step_   = 1.0;
    init();
}


//-----------------------------------------------------------------------------


void Viewer_2D::init()
{
    // OpenGL state
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glColor4f(0.0, 0.0, 0.0, 1.0);
    glDisable( GL_DITHER );
    glEnable( GL_DEPTH_TEST );

    // set projection matrix
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluOrtho2D(-1.1, 1.1, -1.1, 1.1);

    // set modelview matrix
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    // some performance settings
    glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE );
    glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE );

    // surface material
    GLfloat mat_a[] = {0.2, 0.2, 0.2, 1.0};
    GLfloat mat_d[] = {0.4, 0.4, 0.4, 1.0};
    GLfloat mat_s[] = {0.9, 0.9, 0.9, 1.0};
    GLfloat shine[] = {100.0};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   mat_a);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   mat_d);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  mat_s);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);

    // lighting
    glLoadIdentity();
    GLfloat pos1[] = { 0.2, 0.1, 0.3, 0.0};
    GLfloat pos2[] = {-0.2, 0.1, 0.3, 0.0};
    GLfloat  col[] = { 1.0, 1.0, 1.0, 1.0};

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0,GL_POSITION, pos1);
    glLightfv(GL_LIGHT0,GL_DIFFUSE,  col);
    glLightfv(GL_LIGHT0,GL_SPECULAR, col);

    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1,GL_POSITION, pos2);
    glLightfv(GL_LIGHT1,GL_DIFFUSE,  col);
    glLightfv(GL_LIGHT1,GL_SPECULAR, col);

    glEnable(GL_LIGHTING);
}


//-----------------------------------------------------------------------------


void Viewer_2D::reshape(int _w, int _h)
{
    // adjust viewport
    width_  = _w;
    height_ = _h;
    glViewport(0, 0, _w, _h);

    // adjust projection
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    if (width_ > height_)
        gluOrtho2D(-1.1*(float)width_/(float)height_, 1.1*(float)width_/(float)height_, 
                   -1.1, 1.1);
    else
        gluOrtho2D(-1.1, 1.1,
                   -1.1*(float)height_/(float)width_, 1.1*(float)height_/(float)width_);
    glMatrixMode( GL_MODELVIEW );

    // redraw scene
    glutPostRedisplay();
}


//-----------------------------------------------------------------------------


void Viewer_2D::keyboard(int key, int x, int y)
{
    switch (key)
    {
        // toggle automatic animation on/off
        case ' ':
        {
            animate_ = !animate_;
            toggle_idle(animate_);
            glutPostRedisplay();
            break;
        }
        // adjust time step
        case 'T':
        case 't':
        {
            time_step_ *= (key=='T' ? 2.0 : 0.5);
            glutPostRedisplay();
            break;
        }
        // do a single time step
        case 's':
        {
            idle();
            break;
        }
        // parent's job
        default:
        {
            GLUT_viewer::keyboard(key, x, y);
            break;
        }
    }
}


//-----------------------------------------------------------------------------


void Viewer_2D::idle()
{
    time_integration(time_step_);
    glutPostRedisplay();
}


//-----------------------------------------------------------------------------


void Viewer_2D::draw()
{
    // draw some status text
    glDisable(GL_LIGHTING);
    glColor3f(1,0,0);
    std::ostringstream oss;
    oss.str("");
    oss << "Time-Step: " << time_step_;
    glText(20, height_-20, oss.str());

    // draw play/pause symbol
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, width_, 0, height_);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    if (animate_)
    {
        glBegin(GL_TRIANGLES);
        glVertex2i( width_-50, height_-20 );
        glVertex2i( width_-50, height_-40 );
        glVertex2i( width_-30, height_-30 );
        glEnd();
    }
    else
    {
        glBegin(GL_QUADS);
        glVertex2i( width_-50, height_-20 );
        glVertex2i( width_-50, height_-40 );
        glVertex2i( width_-45, height_-40 );
        glVertex2i( width_-45, height_-20 );
        glVertex2i( width_-40, height_-20 );
        glVertex2i( width_-40, height_-40 );
        glVertex2i( width_-35, height_-40 );
        glVertex2i( width_-35, height_-20 );
        glEnd();
    }
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


//-----------------------------------------------------------------------------


vec2 Viewer_2D::pick(int _x, int _y)
{
    // query current OpenGL viewing settings
    GLdouble projection[16], modelview[16];
    GLint    viewport[4];
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetIntegerv(GL_VIEWPORT, viewport);

    // gluUnproject gives us the point on the near plane
    GLdouble wx=_x, wy=height_-_y, ox, oy, oz;
    gluUnProject(wx, wy, 0.0,
                 modelview,
                 projection,
                 viewport,
                 &ox, &oy, &oz );

    return vec2(ox, oy);
}


//=============================================================================
