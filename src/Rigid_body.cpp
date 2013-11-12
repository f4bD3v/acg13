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

#include "Rigid_body.h"
#include <utils/gl.h>


//== IMPLEMENTATION ========================================================== 


Rigid_body::Rigid_body( const std::vector<vec2>& _points, float _mass )
{
    // copy points and mass
    points = _points;
    mass   = _mass;

    // compute center of mass
    position[0] = position[1] = 0.0;
    for (unsigned int i=0; i<points.size(); ++i)
        position += points[i];
    position /= (float) points.size();

    // compute points in local coordinate system
    r.resize(points.size());
    for (unsigned int i=0; i<points.size(); ++i)
        r[i] = points[i] - position;

    // compute moment of inertia
    inertia = 0.0;
    float mi = mass / (float) points.size();
    for (unsigned int i=0; i<points.size(); ++i)
        inertia += mi * norm(r[i]) * norm(r[i]);

    // initialize rotation
    orientation = 0.0;

    // initialize velocities & forces
    linear_velocity  = force  = vec2(0.0, 0.0);
    angular_velocity = torque = 0.0;
}


//-----------------------------------------------------------------------------


void Rigid_body::update_points()
{
    const float s = sin(orientation), c = cos(orientation);

    for (unsigned int i=0; i<points.size(); ++i)
    {
        points[i] = position + vec2( c*r[i][0] + s*r[i][1],
                                    -s*r[i][0] + c*r[i][1] );
    }
}


//-----------------------------------------------------------------------------


void Rigid_body::draw() const
{
    if (!points.empty())
    {
        // draw vertices
        glEnable(GL_LIGHTING);
        glShadeModel(GL_SMOOTH);
        glEnable(GL_COLOR_MATERIAL);
        glColor3f(0.0, 0.6, 0.0);
        for (unsigned int i=0; i<points.size(); ++i)
        {
            glPushMatrix();
            glTranslated( points[i][0], points[i][1], 0.0 );
            glutSolidSphere( 0.015, 20, 20 );
            glPopMatrix();
        }
        glDisable(GL_COLOR_MATERIAL);

        // draw edges
        glDisable(GL_LIGHTING);
        glLineWidth(2.0);
        glColor3f(0.0, 0.0, 0.0);
        glBegin(GL_LINE_LOOP);
        for (unsigned int j=0; j<points.size(); ++j)
        {
            glVertex2fv( points[j].data() );
        }
        glEnd();
    }
}


//=============================================================================
