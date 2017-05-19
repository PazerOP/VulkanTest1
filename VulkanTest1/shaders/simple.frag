#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "simple_shared.glsl"
#include "hsp.glsl"

layout(constant_id = ID_SMOOTH_ANIMATION) const bool SMOOTH_ANIMATION = false;
layout(constant_id = ID_VERTEXCOLOR) const bool VERTEXCOLOR = true;
layout(constant_id = TEXTURE_MODE_START + BINDING_MATERIAL_BASETEXTURE) const int baseTextureMode = TEXTURE_MODE_INVALID;

//layout(set = SET_MATERIAL, binding = BINDING_MATERIAL_CONSTANTS) uniform MaterialConstants
//{
//	int baseTextureMode;
//} material;

layout(set = SET_MATERIAL, binding = BINDING_MATERIAL_BASETEXTURE) uniform sampler2D baseTexture2D;
layout(set = SET_MATERIAL, binding = BINDING_MATERIAL_BASETEXTURE) uniform sampler3D baseTexture3D;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
	vec3 hsp = toHSP(fragColor.rgb);
	hsp.x = mod(hsp.x + (frame.time / 10), 1);
	//hsp.y = 1;
	//hsp.z = 1;
	vec3 rgb = toRGB(hsp);
	rgb.r = clamp(rgb.r, 0, 1);
	rgb.g = clamp(rgb.g, 0, 1);
	rgb.b = clamp(rgb.b, 0, 1);

	//outColor = vec4(rgb, 1.0);
	//outColor = vec4(fragTexCoord, 0.0, 1.0);
	
	if (baseTextureMode == TEXTURE_MODE_3D)
	{	
		const float progress = Remap(0, 1, -1, 1, sin(frame.time / 10));
		if (SMOOTH_ANIMATION)
		{
			int frameCount = textureSize(baseTexture3D, 0).z;
			float frame = mix(0, frameCount - 1, progress);

			float frame0 = floor(frame);
			float frame1 = ceil(frame);

			vec4 frame0Sample = texture(baseTexture3D, vec3(fragTexCoord, Remap(0, 1, 0, frameCount - 1, frame0)));
			vec4 frame1Sample = texture(baseTexture3D, vec3(fragTexCoord, Remap(0, 1, 0, frameCount - 1, frame1)));

			outColor = mix(frame0Sample, frame1Sample, Remap(0, 1, frame0, frame1, frame));
		}
		else
		{
			outColor = texture(baseTexture3D, vec3(fragTexCoord, progress));
		}

		// Smooth alpha
		{
			vec2 delta = vec2(dFdx(fragTexCoord.x), dFdy(fragTexCoord.y));

			float leftAlpha = texture(baseTexture3D, vec3(fragTexCoord.x - delta.x, fragTexCoord.y, progress)).a;
			float upAlpha = texture(baseTexture3D, vec3(fragTexCoord.x, fragTexCoord.y + delta.y, progress)).a;
			float rightAlpha = texture(baseTexture3D, vec3(fragTexCoord.x + delta.x, fragTexCoord.y, progress)).a;
			float downAlpha = texture(baseTexture3D, vec3(fragTexCoord.x, fragTexCoord.y - delta.y, progress)).a;

			float minAlpha = min(leftAlpha, min(upAlpha, min(rightAlpha, min(downAlpha, outColor.a))));
			float maxAlpha = max(leftAlpha, max(upAlpha, max(rightAlpha, max(downAlpha, outColor.a))));
			outColor.a = (minAlpha + maxAlpha) / 2;
		}
	}
	else if (baseTextureMode == TEXTURE_MODE_2D)
		outColor = texture(baseTexture2D, fragTexCoord);
	else
		outColor = vec4(mod(frame.time, 1), 0, 0, 1);

	if (VERTEXCOLOR)
		outColor *= vec4(rgb, 1);

}