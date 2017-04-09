#pragma once

#include <math.h>

namespace
{

struct Vec2
{
	float x;
	float y;
};

} // namespace

inline bool tryCalculateXPositionAtHeight(float h, Vec2 p, Vec2 v, float G, float w, float& xPosition)
{
	// Integrate from third equation of motion, reference: http://physics.info/kinematics-calculus/
	float endVSq = v.y*v.y + 2 * G*(h - p.y); // v^2 = u^2 + 2as
	bool success = (endVSq >= 0); // Path does not intersect at h if endVSq is below zero
	if (success)
	{
		// Path will intersect with h, derive elapsed time
		float endV = sqrtf(endVSq);
		float t = (h - p.y) / (v.y + (endV - v.y) * 0.5f);

		// Find resulting x coordinate at time t; horizontal velocity is not affected by gravity
		float unboundedX = v.x * t + p.x;

		// Account for lossless bounces between [0..w]
		xPosition = fmodf(unboundedX, w * 2); // Wrap to range between [-2w..2w]
		xPosition = fabs(xPosition); // Reflect off left wall
		if (xPosition > w)
		{
			xPosition = w - (xPosition - w); // Reflect off right wall
		}
	}
	return success;
}
