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


#include "Rigid_body_viewer.h"
#include <utils/gl.h>

//=============================================================================


int main(int argc, char **argv)
{
    glutInit(&argc, argv);

    Rigid_body_viewer window("Rigid Bodies", 1024, 768);

    glutMainLoop();
}


//=============================================================================
