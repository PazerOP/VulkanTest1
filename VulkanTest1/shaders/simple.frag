#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "simple_shared.glsl"

layout(constant_id = ID_SMOOTH_ANIMATION) const bool SMOOTH_ANIMATION = true;
layout(constant_id = TEXTURE_MODE_START + BINDING_MATERIAL_BASETEXTURE) const int baseTextureMode = TEXTURE_MODE_INVALID;

layout(set = SET_MATERIAL, binding = BINDING_MATERIAL_BASETEXTURE) uniform sampler2D baseTexture2D;
layout(set = SET_MATERIAL, binding = BINDING_MATERIAL_BASETEXTURE) uniform sampler3D baseTexture3D;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

const vec3 HSPC = vec3(0.299f, 0.587f, 0.114f);

vec3 toHSP(vec3 rgb)
{
	vec3 retVal;

	if (rgb.r == rgb.g && rgb.r == rgb.b)
		retVal.x = 0;
	else if (rgb.r >= rgb.g && rgb.r >= rgb.b)	// r is largest
	{
		if (rgb.b >= rgb.g)
		{
			retVal.x = 6.0f / 6.0f - 1.0f / 6.0f * (rgb.b - rgb.g) / (rgb.r - rgb.g);
			retVal.y = 1.0f - rgb.g / rgb.r;
		}
		else
		{
			retVal.x = 0.0f / 6.0f + 1.0f / 6.0f * (rgb.g - rgb.b) / (rgb.r - rgb.b);
			retVal.y = 1.0f - rgb.b / rgb.r;
		}
	}
	else if (rgb.g >= rgb.r && rgb.g >= rgb.b)	// g is largest
	{
		if (rgb.r >= rgb.b)
		{
			retVal.x = 2.0f / 6.0f - 1.0f / 6.0f * (rgb.r - rgb.b) / (rgb.g - rgb.b);
			retVal.y = 1.0f - rgb.b / rgb.g;
		}
		else
		{
			retVal.x = 2.0f / 6.0f + 1.0f / 6.0f * (rgb.b - rgb.r) / (rgb.g - rgb.r);
			retVal.y = 1.0f - rgb.r / rgb.g;
		}
	}
	else										// b is largest
	{
		if (rgb.g >= rgb.r)
		{
			retVal.x = 4.0f / 6.0f - 1.0f / 6.0f * (rgb.g - rgb.r) / (rgb.b - rgb.r);
			retVal.y = 1.0f - rgb.r / rgb.b;
		}
		else
		{
			retVal.x = 4.0f / 6.0f + 1.0f / 6.0f * (rgb.r - rgb.g) / (rgb.b - rgb.g);
			retVal.y = 1.0f - rgb.g / rgb.b;
		}
	}

	retVal.z = sqrt(
		pow(rgb.r, 2) * HSPC.r +
		pow(rgb.g, 2) * HSPC.g +
		pow(rgb.b, 2) * HSPC.b);

	return retVal;
}

vec3 toRGB(vec3 hsp)
{
	vec3 retVal;
	float minOverMax = 1 - hsp.y;

	if (hsp.x < (1 / 6.0f))			// R>G>B
	{
		hsp.x = 6 * (hsp.x - 0 / 6.0f);
		if (minOverMax > 0)
		{
			const float part = 1 + hsp.x * (1 / minOverMax - 1);
			retVal.b = hsp.z / sqrt(HSPC.r / minOverMax / minOverMax + HSPC.g * pow(part, 2) + HSPC.b);
			retVal.r = retVal.b / minOverMax;
			retVal.g = retVal.b + hsp.x * (retVal.r - retVal.b);
		}
		else
		{
			retVal.r = sqrt(pow(hsp.z, 2) / (HSPC.r + HSPC.g * pow(hsp.x, 2)));
			retVal.g = retVal.r * hsp.x;
			retVal.b = 0;
		}
	}
	else if (hsp.x < (2 / 6.0f))	// G>R>B
	{
		hsp.x = 6 * (-hsp.x + 2 / 6.0f);
		if (minOverMax > 0)
		{
			const float part = 1 + hsp.x * (1 / minOverMax - 1);
			retVal.b = hsp.z / sqrt(HSPC.g / minOverMax / minOverMax + HSPC.r * pow(part, 2) + HSPC.b);
			retVal.g = retVal.b / minOverMax;
			retVal.r = retVal.b + hsp.x * (retVal.g - retVal.b);
		}
		else
		{
			retVal.g = sqrt(pow(hsp.z, 2) / (HSPC.g + HSPC.r * pow(hsp.x, 2)));
			retVal.r = retVal.g * hsp.x;
			retVal.b = 0;
		}
	}
	else if (hsp.x < (3 / 6.0f))	// G>B>R
	{
		hsp.x = 6 * (hsp.x - 2 / 6.0f);
		if (minOverMax > 0)
		{
			const float part = 1 + hsp.x * (1 / minOverMax - 1);
			retVal.r = hsp.z / sqrt(HSPC.g / minOverMax / minOverMax + HSPC.b * pow(part, 2) + HSPC.r);
			retVal.g = retVal.r / minOverMax;
			retVal.b = retVal.r + hsp.x * (retVal.g - retVal.r);
		}
		else
		{
			retVal.g = sqrt(pow(hsp.z, 2) / (HSPC.g + HSPC.b * pow(hsp.x, 2)));
			retVal.b = retVal.g * hsp.x;
			retVal.r = 0;
		}
	}
	else if (hsp.x < (4 / 6.0f))	// B>G>R
	{
		hsp.x = 6 * (-hsp.x + 4 / 6.0f);
		if (minOverMax > 0)
		{
			const float part = 1 + hsp.x * (1 / minOverMax - 1);
			retVal.r = hsp.z / sqrt(HSPC.b / minOverMax / minOverMax + HSPC.g * pow(part, 2) + HSPC.r);
			retVal.b = retVal.r / minOverMax;
			retVal.g = retVal.r + hsp.x * (retVal.b - retVal.r);
		}
		else
		{
			retVal.b = sqrt(pow(hsp.z, 2) / (HSPC.b + HSPC.g * pow(hsp.x, 2)));
			retVal.g = retVal.b * hsp.x;
			retVal.r = 0;
		}
	}
	else if (hsp.x < (5 / 6.0f))	// B>R>G
	{
		hsp.x = 6 * (hsp.x - 4 / 6.0f);
		if (minOverMax > 0)
		{
			const float part = 1 + hsp.x * (1 / minOverMax - 1);
			retVal.g = hsp.z / sqrt(HSPC.b / minOverMax / minOverMax + HSPC.r * pow(part, 2) + HSPC.g);
			retVal.b = retVal.g / minOverMax;
			retVal.r = retVal.g + hsp.x * (retVal.b - retVal.g);
		}
		else
		{
			retVal.b = sqrt(pow(hsp.z, 2) / (HSPC.b + HSPC.r * pow(hsp.x, 2)));
			retVal.r = retVal.b * hsp.x;
			retVal.g = 0;
		}
	}
	else							// R>B>G
	{
		hsp.x = 6 * (-hsp.x + 6 / 6.0f);
		if (minOverMax > 0)
		{
			const float part = 1 + hsp.x * (1 / minOverMax - 1);
			retVal.g = hsp.z / sqrt(HSPC.r / minOverMax / minOverMax + HSPC.b * pow(part, 2) + HSPC.g);
			retVal.r = retVal.g / minOverMax;
			retVal.b = retVal.g + hsp.x * (retVal.r - retVal.g);
		}
		else
		{
			retVal.r = sqrt(pow(hsp.z, 2) / (HSPC.r + HSPC.b * pow(hsp.x, 2)));
			retVal.b = retVal.r * hsp.x;
			retVal.g = 0;
		}
	}

	return retVal;
}

void main()
{
	vec3 hsp = toHSP(fragColor.rgb);
	hsp.x = mod(hsp.x + (frame.time / 10), 1);
	hsp.y = hsp.z = 1;
	vec3 rgb = toRGB(hsp);
	rgb.r = clamp(rgb.r, 0, 1);
	rgb.g = clamp(rgb.g, 0, 1);
	rgb.b = clamp(rgb.b, 0, 1);

	//outColor = vec4(rgb, 1.0);
	//outColor = vec4(fragTexCoord, 0.0, 1.0);
	
	if (baseTextureMode == TEXTURE_MODE_3D)
	{	
		const float progress = Remap(0, 1, -1, 1, sin(frame.time));
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
	}
	else if (baseTextureMode == TEXTURE_MODE_2D)
		outColor = texture(baseTexture2D, fragTexCoord);
	else
		outColor = vec4(mod(frame.time, 1), 0, 0, 1);
}