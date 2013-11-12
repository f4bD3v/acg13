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

#include "Rigid_body_viewer.h"
#include <utils/gl.h>


//== IMPLEMENTATION ==========================================================

float cross(vec2 a, vec2 b){
    return a[0]*b[1]-a[1]*b[0];
}

Rigid_body_viewer::Rigid_body_viewer(const char* _title, int _width, int _height)
    : Viewer_2D(_title, _width, _height)
{
    animate_             = false;
    time_step_           = 0.001;
    mass_                = 0.5;
    damping_linear_      = 0.1;
    damping_angular_     = 0.0001;
    spring_stiffness_    = 100.0;
    spring_damping_      = 5.0;

    mouse_spring_.active = false;
}


//-----------------------------------------------------------------------------


void Rigid_body_viewer::keyboard(int key, int x, int y)
{
    switch (key)
    {
    case '1':
    {
        std::vector<vec2> p;
        p.push_back( vec2(-0.6, -0.6) );
        p.push_back( vec2(-0.4, -0.6) );
        p.push_back( vec2(-0.4, -0.4) );
        p.push_back( vec2(-0.6, -0.4) );

        body_ = Rigid_body(p, mass_);
        body_.linear_velocity = vec2(5.0, 5.0);
        glutPostRedisplay();
        break;
    }
    case '2':
    {
        std::vector<vec2> p;
        p.push_back( vec2(-0.3, -0.1) );
        p.push_back( vec2(-0.1, -0.1) );
        p.push_back( vec2( 0.1, -0.1) );
        p.push_back( vec2( 0.3, -0.1) );
        p.push_back( vec2( 0.3,  0.1) );
        p.push_back( vec2( 0.1,  0.1) );
        p.push_back( vec2(-0.1,  0.1) );
        p.push_back( vec2(-0.3,  0.1) );

        body_ = Rigid_body(p, mass_);

        glutPostRedisplay();
        break;
    }
    case '3':
    {
        std::vector<vec2> p;
        p.push_back( vec2(-0.5,  0.1) );
        p.push_back( vec2(-0.5,  0.0) );
        p.push_back( vec2( 0.0,  0.0) );
        p.push_back( vec2( 0.0, -0.3) );
        p.push_back( vec2( 0.1, -0.3) );
        p.push_back( vec2( 0.1,  0.0) );
        p.push_back( vec2( 0.3,  0.0) );
        p.push_back( vec2( 0.3,  0.1) );

        body_ = Rigid_body(p, mass_);

        glutPostRedisplay();
        break;
    }
        // let parent class do the work
    default:
    {
        Viewer_2D::keyboard(key, x, y);
        break;
    }
    }
}


//-----------------------------------------------------------------------------


void Rigid_body_viewer:: mouse(int _button, int _state, int _x, int _y)
{
    // need points
    if (body_.points.empty())
        return;

    // mouse button release destroys current mouse spring
    if (_state == GLUT_UP)
    {
        mouse_spring_.active = false;
        return;
    }

    // mouse button press generates new mouse spring
    else if (_state == GLUT_DOWN)
    {
        // get point under mouse cursor
        vec2 p = pick(_x, _y);

        // find closest body point
        unsigned int i, imin;
        float dmin = FLT_MAX;
        for (i=0; i<body_.points.size(); ++i)
        {
            float d = distance(p, body_.points[i]);
            if (d < dmin)
            {
                dmin = d;
                imin = i;
            }
        }

        // setup the mouse spring
        mouse_spring_.active = true;
        mouse_spring_.particle_index = imin;
        mouse_spring_.mouse_position = p;
    }

    glutPostRedisplay();
}


//-----------------------------------------------------------------------------


void Rigid_body_viewer:: motion(int _x, int _y)
{
    if (mouse_spring_.active)
    {
        // update mouse position
        mouse_spring_.mouse_position = pick(_x, _y);
        glutPostRedisplay();
    }
}


//-----------------------------------------------------------------------------


void Rigid_body_viewer:: draw()
{
    // parent's status text
    Viewer_2D::draw();

    // draw walls
    glDisable(GL_LIGHTING);
    glLineWidth(1.0);
    glColor3f(0.5,0.5,0.5);
    glBegin(GL_LINE_STRIP);
    glVertex2f( -1.0,  1.0 );
    glVertex2f( -1.0, -1.0 );
    glVertex2f(  1.0, -1.0 );
    glVertex2f(  1.0,  1.0 );
    glVertex2f( -1.0,  1.0 );
    glEnd();

    // draw rigid body
    body_.draw();

    // draw mouse spring
    if (mouse_spring_.active)
    {
        glLineWidth(5.0);
        glColor3f(1,0,0);
        glBegin(GL_LINES);
        glVertex2fv( body_.points[ mouse_spring_.particle_index ].data() );
        glVertex2fv( mouse_spring_.mouse_position.data() );
        glEnd();
    }
}


//-----------------------------------------------------------------------------


void Rigid_body_viewer::compute_forces()
{
    /** \todo Compute all forces acting on the rigid body
     \li clear all forces
     \li add gravity
     \li add damping to linear and angular movement
     \li add the mouse spring force
     */

    /// clear all forces
    body_.force = vec2(0.0f, 0.0f);
    body_.torque = 0.0f;

    /// add gravity
    body_.force -= vec2(0.0f, body_.mass * 9.81f);

    /// add damping to linear movement
    body_.force -= damping_linear_ * body_.linear_velocity;

    /// add damping to angular movement
    body_.torque -= damping_linear_ * body_.angular_velocity;

    /// add the mouse spring force
    if (mouse_spring_.active) {
        // get positions
        vec2 center = body_.position;
        vec2 mouse = mouse_spring_.mouse_position;
        // linear part : do as in the previous exercise for the linear part
        vec2 diff = center - mouse;
        float dist = norm(diff);
        vec2 drct = diff / dist;
        float intensity = spring_stiffness_ * dist;
        intensity += spring_damping_ * dot(body_.linear_velocity, diff) / dist;
        body_.force -= intensity * drct;
        // angular part
        vec2 point = body_.points[mouse_spring_.particle_index];
        vec2 rPerp = perp(point - center);
        vec2 force = mouse - point;
        body_.torque += dot(rPerp, force);
    }

}


//-----------------------------------------------------------------------------


void Rigid_body_viewer::impulse_based_collisions()
{
float planes[4][3] = {
            {  0.0,  1.0, 1.0 },
            {  0.0, -1.0, 1.0 },
            {  1.0,  0.0, 1.0 },
            { -1.0,  0.0, 1.0 }
    };

    float R = 0.9f;
/*
    for(unsigned int i =0; i<4; i++){
        vec2 n(planes[i][0],planes[i][1]);
        vec2 p = -planes[i][2]*n;
        for(unsigned int j = 0; j<body_.points.size(); j++){
            vec2 xj = body_.points[j];
            // Collision detection
            float d = dot(n,(xj-p));
            if(!(d>0)){
                // Collision response
                vec2 vj = body_.linear_velocity + body_.angular_velocity*perp(body_.r[j]);
                float impuls = -(1+R)*dot(n,vj)/(1/body_.mass + pow(cross(body_r[j],n),2)/body_.inertia);
                body_.linear_velocity += impuls/body_.mass*n;
                body_.angular_velocity += cross(body_.r[j],impuls*n)/body_.inertia;
            }
        }

    }*/
}


//-----------------------------------------------------------------------------


void Rigid_body_viewer::time_integration(float dt)
{
    // compute all forces
    compute_forces();

    /** \todo Implement explicit Euler time integration
     \li update position and orientation
     \li update linear and angular velocities
     \li call update_points() at the end to compute the new particle positions
     */

    // body is assigned mass in constructor
    // body_.position (center of gravity), body._inertia are computed in the constructor
    // all values needed for the Simulation have been precomputed in the constructor
    // linear velocity and angular velocity are initizalized to 0, update first
    body_.angular_velocity += dt * body_.torque/body_.inertia;
    body_.linear_velocity += dt * body_.force/body_.mass;
    body_.position += dt * body_.linear_velocity;
    body_.orientation += dt * body_.angular_velocity;
    // call update_points() at the end to compute the new particle positions
    body_.update_points();
    // handle collisions
    impulse_based_collisions();
}


//=============================================================================
