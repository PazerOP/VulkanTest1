// If one or more of these uniform buffers aren't referenced anywhere
// in a shader and aren't completely optimized away because of that,
// I will be disappointed.

#include "interop.h"

#define M_PI 3.14159265358979323846f

// Remaps t from [x, y] to [a, b]
float Remap(float a, float b, float x, float y, float t)
{
	return mix(a, b, (t - x) / (y - x));
}
vec2 Remap(vec2 a, vec2 b, vec2 x, vec2 y, vec2 t)
{
	return mix(a, b, (t - x) / (y - x));
}

layout(set = SET_FRAMEVIEW, binding = 0) uniform FrameConstants
{
	float time;
	float dt;
} frame;

layout(set = SET_FRAMEVIEW, binding = 1) uniform ViewConstants
{
	vec2 camPos;
	mat4 view;
	mat4 orthoProj;
} view;

layout(set = SET_OBJECT, binding = 0) uniform ObjectConstants
{
	mat4 modelToWorld;
} object;

#define PARAM_ANIMATION_FRAME_BLENDING(id) layout(constant_id = id) const bool ANIMATION_FRAME_BLENDING = false