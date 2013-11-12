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

#ifndef GL_TEXT_H
#define GL_TEXT_H

//== INCLUDES =================================================================

#include "utils/gl.h"
#include <string>

//=============================================================================


/// A small helper function to draw text at positin (x,y) with OpenGL
void glText(int x, int y, const std::string& _text);


//=============================================================================
#endif
//=============================================================================
