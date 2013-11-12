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

#ifndef GL_WRAPPERS_H
#define GL_WRAPPERS_H

//== INCLUDES =================================================================


// Include files for OpenGL and GLUT have different paths under MacOS...

// Mac OS X
#ifdef __APPLE__
#  include <GLUT/glut.h>
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>

// Windows
#elif _WIN32
#  include <stdlib.h>
#  include <GL/glut.h>
#  include <GL/gl.h>
#  include <GL/glu.h>

// Unix
#else
#  include <GL/glut.h>
#  include <GL/gl.h>
#  include <GL/glu.h>
#endif


//=============================================================================
#endif // GL_WRAPPERS_HH defined
//=============================================================================
