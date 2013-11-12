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

#ifndef VIEWER_2D_H
#define VIEWER_2D_H


//== INCLUDES =================================================================

#include "utils/GLUT_viewer.h"
#include "utils/vec2.h"
#include <string>
#include <vector>


//== CLASS DEFINITION =========================================================


/** \class Viewer_2D Viewer_2D.h <utils/Viewer_2D.h>
 A 2D viewer for simulations.
 This base class provides the basic functionality to run a dynamic simulation.
 The viewer classes of the actual exercises will derive from this class.
 By spressing the space bar the user can turn on/off the simulation. 
 When in "play" mode this class automatically triggers the time_integration() 
 function, which is overloaded by the derived classes.
 */

class Viewer_2D : public GLUT_viewer
{
public:
    /// constructor
    Viewer_2D(const char* _title, int _width, int _height);

protected: // GUI functions
    /// initialize OpenGL stuff
    virtual void init();

    /// render the scene
    virtual void draw();

    /// handle resizing of the window
    virtual void reshape(int w, int h);

    /// handle keyboard events
    virtual void keyboard(int key, int x, int y);

    /// this function triggers the time_integration() function
    virtual void idle();

    /// pick a 2D point by mouse clicking
    vec2 pick(int _x, int _y);

protected: // simulation functions
    /// purely virtual time integration function, will be implemented by derived classes
    virtual void time_integration(float dt) = 0;

protected: // simulation settings
    /// is animation on/off?
    bool animate_;

    /// value of time-step
    float time_step_;
};


//=============================================================================
#endif
//=============================================================================
