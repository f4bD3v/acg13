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

#include <cstdlib>
#include <utils/gl.h>
#include <utils/GLUT_viewer.h>


//== IMPLEMENTATION ========================================================== 


std::map<int, GLUT_viewer*>  GLUT_viewer::windows__;


//-----------------------------------------------------------------------------


GLUT_viewer::
GLUT_viewer(const char* _title, int _width, int _height)
: width_(_width), height_(_height), fullscreen_(false)
{
    // create window
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE | GLUT_ALPHA);
    glutInitWindowSize(_width, _height);
    windowID_ = glutCreateWindow(_title);
    windows__[windowID_] = this;

    // register callbacks
    glutDisplayFunc(display__);
    glutKeyboardFunc(keyboard__);
    glutSpecialFunc(special__);
    glutMouseFunc(mouse__);
    glutMotionFunc(motion__);
    glutPassiveMotionFunc(passivemotion__);
    glutReshapeFunc(reshape__);
    glutVisibilityFunc(visibility__);
}


//-----------------------------------------------------------------------------


GLUT_viewer::
~GLUT_viewer()
{
    glutDestroyWindow(windowID_);
}


//-----------------------------------------------------------------------------


GLUT_viewer* GLUT_viewer::current_window() {
    return windows__[glutGetWindow()];
}

void GLUT_viewer::toggle_idle(bool _b) {
    glutIdleFunc(_b ? idle__ : NULL);
}

void GLUT_viewer::display__(void) {
    current_window()->display();
}

void GLUT_viewer::idle__(void) {
    current_window()->idle();
}

void GLUT_viewer::keyboard__(unsigned char key, int x, int y) {
    current_window()->keyboard((int)key, x, y);
}

void GLUT_viewer::motion__(int x, int y) {
    current_window()->motion(x, y);
}

void GLUT_viewer::mouse__(int button, int state, int x, int y) {
    current_window()->mouse(button, state, x, y);
}

void GLUT_viewer::passivemotion__(int x, int y) {
    current_window()->passivemotion(x, y);
}

void GLUT_viewer::reshape__(int w, int h) {
    current_window()->reshape(w, h);
}

void GLUT_viewer::special__(int key, int x, int y) {
    current_window()->special(key, x, y);
}

void GLUT_viewer::visibility__(int visible) {
    current_window()->visibility(visible);
}

void GLUT_viewer::processmenu__(int id) {
    current_window()->processmenu(id);
}


//-----------------------------------------------------------------------------


void GLUT_viewer::keyboard(int key, int x, int y)
{
    switch (key)
    {
        case 'f':
        {
            if (!fullscreen_)
            {
                bak_left_   = glutGet(GLUT_WINDOW_X);
                bak_top_    = glutGet(GLUT_WINDOW_Y);
                bak_width_  = glutGet(GLUT_WINDOW_WIDTH);
                bak_height_ = glutGet(GLUT_WINDOW_HEIGHT);
                glutFullScreen();
                fullscreen_ = true;
            }
            else
            {
                glutReshapeWindow(bak_width_, bak_height_);
                glutPositionWindow(bak_left_, bak_top_);
                fullscreen_ = false;
            }
            break;
        }
        case 27:
        {
            exit(0);
            break;
        }
    }
}


//-----------------------------------------------------------------------------


void GLUT_viewer::display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw();
    glutSwapBuffers();
}


//=============================================================================
