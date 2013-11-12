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

#ifndef VEC2_H
#define VEC2_H


//== INCLUDES =================================================================

#include <assert.h>
#include <iostream>
#include <float.h>
#include <math.h>


//== CLASS DEFINITION =========================================================

/// \file vec2.h

/** \class vec2 vec2.h <utils/vec2.h>
 A simple class for 2D vectors that offers most vector-vector and
 scalar-vector operations. The syntax is similar to GLSL. Besides the
 typical operators there are also these function:
 <ul>
 <li> dot()
 <li> norm()
 <li> normalize()
 <li> distance()
 <li> perp()
 </ul>
 \sa vec2.h
 */
class vec2
{
private:
    
    /// internal data
    float data_[2];
    
    
public:
    
    /// default constructor
    vec2() {}
    
    /// construct with x and y coordinate
    vec2(const float x, const float y)
    { 
        data_[0]=x; data_[1]=y; 
    }
    
    /// access i'th component
    float& operator[](unsigned int i)
    {
        assert(i<2);
        return data_[i];
    }
    
    /// access i'th component
    float operator[](unsigned int i) const
    {
        assert(i<2);
        return data_[i];
    }
    
    /// convert to float pointer
    const float* data() const
    { 
		return data_; 
	}
    
    /// self-multiply with scalar
    vec2& operator*=(const float s)
    {
        for (int i=0; i<2; ++i) data_[i] *= s;
        return *this;
    }
    
    /// self-divide by scalar
    vec2& operator/=(const float s)
    {
        for (int i=0; i<2; ++i) data_[i] /= s;
        return *this;
    }
    
    /// subtract vector
    vec2& operator-=(const vec2& v)
    {
        for (int i=0; i<2; ++i) data_[i] -= v.data_[i];
        return *this;
    }
    
    /// add vector
    vec2& operator+=(const vec2& v)
    {
        for (int i=0; i<2; ++i) data_[i] += v.data_[i];
        return *this;
    }
};


//-----------------------------------------------------------------------------


/// negate vector
inline vec2 operator-(const vec2& v)
{
    return vec2(-v[0], -v[1]);
}

/// multiply vector with scalar
inline vec2 operator*(const float s, const vec2& v )
{
    return vec2(v) *= s;
}

/// multiply vector with scalar
inline vec2 operator*(const vec2& v, const float s)
{
    return vec2(v) *= s;
}

/// divide vector by scalar
inline vec2 operator/(const vec2& v, const float s)
{
    return vec2(v) /= s;
}

/// add two vectors
inline vec2 operator+(const vec2& v0, const vec2& v1)
{
    return vec2(v0) += v1;
}

/// subtract two vectors
inline vec2 operator-(const vec2& v0, const vec2& v1)
{
    return vec2(v0) -= v1;
}

/// Euclidean norm of a vector
inline float norm(const vec2& v)
{
    float s(0.0f);
    for (int i=0; i<2; ++i) s += v[i]*v[i];
    return sqrtf(s);
}

/// return normalized version of vector
inline vec2 normalize(const vec2& v)
{
    return vec2(v) /= norm(v);
}

/// distance between two points (norm of difference)
inline float distance(const vec2& v0, const vec2& v1)
{
    vec2 d=v0; d-=v1;
    return norm(d);
}

/// dot product of two vectors
inline float dot(const vec2& v0, const vec2& v1)
{
    float s(0.0f);
    for (int i=0; i<2; ++i) s += v0[i]*v1[i];
    return s;
}

/// perpendicular vector
inline vec2 perp(const vec2& v)
{
    return vec2( v[1], -v[0] );
}

/// output vector
inline std::ostream& operator<<(std::ostream& os, const vec2& v)
{
    os << '(' << v[0] << ", " << v[1] << ')';
    return os;
}


//=============================================================================
#endif
//=============================================================================
