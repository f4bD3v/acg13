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

#ifndef RIGID_BODY_VIEWER_H
#define RIGID_BODY_VIEWER_H


//== INCLUDES =================================================================

#include "Rigid_body.h"
#include <utils/Viewer_2D.h>


//== CLASS DEFINITION =========================================================


/** \class Rigid_body_viewer Rigid_body_viewer.h <02-rigid_bodies/Rigid_body_viewer.h>
 Viewer class for the mass spring exercise.
 */
class Rigid_body_viewer : public Viewer_2D
{
public:
    /// constructor
    Rigid_body_viewer(const char* _title, int _width, int _height);

private: // GUI functions
    /// draw scene
    virtual void draw();

    /// handle keyboard events
    virtual void keyboard(int key, int x, int y);

    /// handle mouse events (used for interactive spring)
    virtual void mouse(int button, int state, int x, int y);

    /// handle mouse move events (used for interactive spring)
    virtual void motion(int x, int y);

private: // simulation functions
    /// compute all external and internal forces
    void compute_forces();

    /// perform one time step using either Euler, Midpoint, or Verlet
    void time_integration(float dt);

    /// perform impulse-based collision handling
    void impulse_based_collisions();

private: // parameter settings
    float mass_; ///< total mass of the rigid body
    float damping_linear_; ///< damping of linear movement
    float damping_angular_; ///< damping of angular movement
    float spring_stiffness_; ///< stiffness of interactive mouse spring
    float spring_damping_;///< damping of interactive mouse spring

private: // simulation data
    Rigid_body body_; ///< the rigid body to be simulated

    /// struct for storing data of the mouse spring
    struct Mouse_spring
    {
        vec2 mouse_position;
        int  particle_index;
        bool active;
    };

    Mouse_spring mouse_spring_; ///< the mouse spring
};


//=============================================================================
#endif
//=============================================================================

