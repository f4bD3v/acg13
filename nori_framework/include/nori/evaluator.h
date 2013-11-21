/*******************************************************************************
 *  evaluator.h
 *******************************************************************************
 *  Copyright (c) 2013 Alexandre Kaspar <alexandre.kaspar@a3.epfl.ch>
 *  For Advanced Computer Graphics, at the LGG / EPFL
 * 
 *        DO NOT REDISTRIBUTE
 ***********************************************/

#if !defined(__EVALUATOR_H)
#define __EVALUATOR_H

#include <nori/bitmap.h>
#include <nori/object.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Scene result evaluator
 *
 * This abstract class represent the test done on the result of a scene rendering.
 */
class Evaluator : public NoriObject {
public:

	/**
	 * \brief Evaluate a result scene bitmap
	 *
	 * \param result
	 *    A 2d rgb bitmap corresponding to the scene rendering output
	 */
	virtual void evaluate(const Bitmap *result) const = 0;

	EClassType getClassType() const { return EEvaluator; }
};

NORI_NAMESPACE_END

#endif /* __EVALUATOR_H */
