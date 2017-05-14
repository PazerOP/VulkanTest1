// If one or more of these uniform buffers aren't referenced anywhere
// in a some shader and aren't completely optimized away because of
// that, I will be disappointed.

layout(binding = 0) uniform FrameConstants
{
	float time;
	float dt;
} frame;

layout(binding = 1) uniform ViewConstants
{
	vec3 camPos;
};

layout(binding = 2) uniform ObjectConstants
{
	mat4 model;
	mat4 view;
	mat4 proj;
} object;