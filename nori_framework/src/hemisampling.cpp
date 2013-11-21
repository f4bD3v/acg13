/*******************************************************************************
 *  hemisampling.cpp
 *******************************************************************************
 *  Copyright (c) 2013 Alexandre Kaspar <alexandre.kaspar@a3.epfl.ch>
 *  For Advanced Computer Graphrm ics, at the LGG / EPFL
 * 
 *        DO NOT REDISTRIBUTE
 ***********************************************/

#include <nori/vector.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Cosine hemisphere sampling
 * 
 * \param sample a 2d uniform sample
 */
extern Vector3f squareToCosineHemisphere(const Point2f &sample) {

        /*
         * Solution from Nori:
     
        Point2f p = squareToUniformDiskConcentric(sample);
        float z = std::sqrt(std::max((float) 0,
                1.0f - p.x() * p.x() - p.y() * p.y()));

        return Vector3f(p.x(), p.y(), z);
     
         */

        /*
         * General solution from
         * http://mathinfo.univ-reims.fr/IMG/pdf/Using_the_modified_Phong_reflectance_model_for_Physically_based_rendering_-_Lafortune.pdf
        */
        float sinTheta = std::sqrt(1.0f - sample.y());
        float cosTheta = std::sqrt(sample.y());
        float phi = 2.0f * M_PI * sample.x();
        float cosPhi, sinPhi;
        sincosf(phi, &sinPhi, &cosPhi);
        return Vector3f(
                sinTheta * cosPhi,
                sinTheta * sinPhi,
                cosTheta
        );
}

NORI_NAMESPACE_END
