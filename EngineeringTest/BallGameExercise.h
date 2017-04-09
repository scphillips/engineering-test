#pragma once

#include <math.h>

struct Vec2
{
	float x;
	float y;
};

inline float reflectValueBetweenBounds(float value, float min, float max)
{
	// Normalize to range starting from minimum wall
	value -= min;
	value = fmodf(value, max * 2); // Wrap to range between [-2w..2w]
	value = fabs(value); // Reflect off left wall
	if (value > max)
	{
		value = max - (value - max); // Reflect off right wall
	}
	// Restore to actual range
	value += min;
	return value;
}

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
		xPosition = reflectValueBetweenBounds(unboundedX, 0, w);
	}
	return success;
}
