#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "simple_shared.glsl"
#include "../hsp.glsl"

layout(constant_id = CID_ANIMATION_FRAME_BLENDING) const bool SMOOTH_ANIMATION = true;
layout(constant_id = CID_VERTEXCOLOR) const bool VERTEXCOLOR = true;
layout(constant_id = CID_SPLIT_PIXELS) const bool SPLIT_PIXELS = false;

layout(constant_id = CID_BASETEXTURE_MODE) const int baseTextureMode = TEXTURE_MODE_INVALID;

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
			const vec2 delta = fwidth(fragTexCoord);

			float minAlpha = 1;
			float maxAlpha = 1;
			
			if (!SPLIT_PIXELS)
			{
				float[9] alphaSamples =
				{
					texture(baseTexture3D, vec3(fragTexCoord.x - delta.x, fragTexCoord.y - delta.y, progress)).a,
					texture(baseTexture3D, vec3(fragTexCoord.x - delta.x, fragTexCoord.y, progress)).a,
					texture(baseTexture3D, vec3(fragTexCoord.x - delta.x, fragTexCoord.y + delta.y, progress)).a,

					texture(baseTexture3D, vec3(fragTexCoord.x, fragTexCoord.y - delta.y, progress)).a,
					outColor.a,
					texture(baseTexture3D, vec3(fragTexCoord.x, fragTexCoord.y + delta.y, progress)).a,

					texture(baseTexture3D, vec3(fragTexCoord.x + delta.x, fragTexCoord.y - delta.y, progress)).a,
					texture(baseTexture3D, vec3(fragTexCoord.x + delta.x, fragTexCoord.y, progress)).a,
					texture(baseTexture3D, vec3(fragTexCoord.x + delta.x, fragTexCoord.y + delta.y, progress)).a,
				};

				minAlpha = alphaSamples[0];
				maxAlpha = alphaSamples[0];
				for (int i = 1; i < 9; i++)
				{
					minAlpha = min(minAlpha, alphaSamples[i]);
					maxAlpha = max(maxAlpha, alphaSamples[i]);
				}
			}

			const vec2 baseTextureSize = textureSize(baseTexture3D, 0).xy;

			const vec2 baseTexturePixelSize = 1 / baseTextureSize;
			vec2 withinPixel = mod(fragTexCoord, baseTexturePixelSize);

			//vec2 distToEdge = (baseTexturePixelSize / 2) - abs(withinPixel - (baseTexturePixelSize / 2));
			const vec2 halfPixelSize = baseTexturePixelSize / 2;
			vec2 distToEdge = halfPixelSize - abs(Remap(-halfPixelSize, halfPixelSize, vec2(0, 0), baseTexturePixelSize, withinPixel));

			//outColor.rgb = mix(vec3(1, 0, 0), outColor.rgb, distToEdge.x);
			//if (distToEdge.x < delta.x || distToEdge.y < delta.y)
			{
				//outColor.rgb = vec3(1, 0, 0);
				vec2 fade = vec2(
					Remap(maxAlpha, minAlpha, delta.x, 0, distToEdge.x),
					Remap(maxAlpha, minAlpha, delta.y, 0, distToEdge.y));
				//outColor.rgb = mix(outColor.rgb, vec3(1, 0, 0), fade);
				outColor.a = min(fade.x, fade.y);
			}

			// See how close we are to the edge of a pixel
#if 0
			// From https://gist.github.com/slembcke/23e1d96d7e3c739caf12
			
			vec2 puv = fragTexCoord * textureSize(baseTexture3D, 0).xy;

			vec2 hfw = fwidth(puv) / 2.0;
			vec2 fl = floor(puv - 0.5) + 0.5;			

			vec2 nnn = (fl + smoothstep(0.5 - hfw, 0.5 + hfw, puv - fl)) / textureSize(baseTexture3D, 0).xy;
			outColor = texture(baseTexture3D, vec3(nnn, progress));
#endif
		}
	}
	else if (baseTextureMode == TEXTURE_MODE_2D)
		outColor = texture(baseTexture2D, fragTexCoord);
	else
		outColor = vec4(mod(frame.time, 1), 0, 0, 1);

	if (VERTEXCOLOR)
		outColor *= vec4(rgb, 1);
}