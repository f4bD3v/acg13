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

#include "utils/gl_text.h"

//=============================================================================


void glText(int x, int y, const std::string& _text)
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // set raster pos
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, (GLfloat) viewport[2], 0.0, (GLfloat) viewport[3]);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glRasterPos2i(x, y);

    // draw characters
    std::string::const_iterator s_it(_text.begin()), s_end(_text.end());
    for (; s_it!=s_end; ++s_it)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *s_it);

    // restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


//=============================================================================
