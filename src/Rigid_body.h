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

#ifndef RIGID_BODY_H
#define RIGID_BODY_H


//== INCLUDES =================================================================

#include <utils/vec2.h>
#include <vector>


//== CLASS DEFINITION =========================================================

/** \class Rigid_body Rigid_body.h <02-rigid_bodies/Rigid_body.h>
 Class for representing a rigid body.
 It represents the rigid body by the position of its center of gravity
 and its orientation (rotation angle). For these two components it also
 stores velocities, forces, and mass/inertia.
 */
class Rigid_body
{
public:
    /// default constructur
    Rigid_body() {}

    /// construct with a set of points and a total mass
    Rigid_body(const std::vector<vec2>& _points, float _mass);

    /// after changing position and orientation, call this function to update particle positions
    void update_points();

    /// render the rigid body
    void draw() const;

public:
    vec2   position; ///< position of the center of gravity (CoG)
    vec2   linear_velocity; ///< linear velocity of CoG
    vec2   force; ///< linear force
    float  mass; ///< total mass of the rigid body

    float  orientation; ///< the current angle of rotation
    float  angular_velocity; ///< angular velocity
    float  torque; ///< torque
    float  inertia; ///< moment of inertia

    std::vector<vec2>  points; ///< vector of particles/points    
    std::vector<vec2>  r; ///< vector of relative point positions
};

//=============================================================================
#endif
//=============================================================================

