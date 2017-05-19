// If one or more of these uniform buffers aren't referenced anywhere
// in a shader and aren't completely optimized away because of that,
// I will be disappointed.

#define M_PI 3.14159265358979323846f

#define SET_FRAMEVIEW 0
#define BINDING_FRAMEVIEW_FRAME 0
#define BINDING_FRAMEVIEW_VIEW 1

#define SET_MATERIAL 1

#define SET_OBJECT 2

// Remaps t from [x, y] to [a, b]
float Remap(float a, float b, float x, float y, float t)
{
	return mix(a, b, (t - x) / (y - x));
}

layout(set = SET_FRAMEVIEW, binding = BINDING_FRAMEVIEW_FRAME) uniform FrameConstants
{
	float time;
	float dt;
} frame;

layout(set = SET_FRAMEVIEW, binding = BINDING_FRAMEVIEW_VIEW) uniform ViewConstants
{
	vec2 camPos;
	mat4 view;
	mat4 orthoProj;
} view;

layout(set = SET_OBJECT, binding = 0) uniform ObjectConstants
{
	mat4 modelToWorld;
} object;