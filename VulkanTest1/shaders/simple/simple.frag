#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "simple_shared.glsl"
#include "../hsp.glsl"

layout(constant_id = 0) const bool _param_FrameBlending = true;
layout(constant_id = 1) const bool _param_VertexColor = true;
layout(constant_id = 2) const bool _param_SplitPixels = false;

const bool SplitPixels = !_param_FrameBlending && _param_SplitPixels;

//layout(set = SET_MATERIAL, binding = BINDING_MATERIAL_CONSTANTS) uniform MaterialConstants
//{
//	int baseTextureMode;
//} material;

layout(set = SET_MATERIAL) uniform sampler2D _tex2D_BaseTexture;
layout(set = SET_MATERIAL) uniform sampler3D _tex3D_BaseTexture;
layout(constant_id = 3) const int _texMode_BaseTexture = TEXTURE_MODE_INVALID;

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
	rgb.rgb = clamp(rgb.rgb, vec3(0, 0, 0), vec3(1, 1, 1));

	//outColor = vec4(rgb, 1.0);
	//outColor = vec4(fragTexCoord, 0.0, 1.0);
	
	if (_texMode_BaseTexture == TEXTURE_MODE_3D)
	{	
		const float progress = Remap(0, 1, -1, 1, sin(frame.time / 10));
		if (_param_FrameBlending)
		{
			int frameCount = textureSize(_tex3D_BaseTexture, 0).z;
			float frame = mix(0, frameCount - 1, progress);

			float frame0 = floor(frame);
			float frame1 = ceil(frame);

			vec4 frame0Sample = texture(_tex3D_BaseTexture, vec3(fragTexCoord, Remap(0, 1, 0, frameCount - 1, frame0)));
			vec4 frame1Sample = texture(_tex3D_BaseTexture, vec3(fragTexCoord, Remap(0, 1, 0, frameCount - 1, frame1)));

			outColor = mix(frame0Sample, frame1Sample, Remap(0, 1, frame0, frame1, frame));
		}
		else
		{
			outColor = texture(_tex3D_BaseTexture, vec3(fragTexCoord, progress));
		}

		// Smooth alpha
		{
			const vec2 delta = fwidth(fragTexCoord);

			float minAlpha = 1;
			float maxAlpha = 1;
			
			if (!SplitPixels)
			{
				float[9] alphaSamples =
				{
					texture(_tex3D_BaseTexture, vec3(fragTexCoord.x - delta.x, fragTexCoord.y - delta.y, progress)).a,
					texture(_tex3D_BaseTexture, vec3(fragTexCoord.x - delta.x, fragTexCoord.y, progress)).a,
					texture(_tex3D_BaseTexture, vec3(fragTexCoord.x - delta.x, fragTexCoord.y + delta.y, progress)).a,

					texture(_tex3D_BaseTexture, vec3(fragTexCoord.x, fragTexCoord.y - delta.y, progress)).a,
					outColor.a,
					texture(_tex3D_BaseTexture, vec3(fragTexCoord.x, fragTexCoord.y + delta.y, progress)).a,

					texture(_tex3D_BaseTexture, vec3(fragTexCoord.x + delta.x, fragTexCoord.y - delta.y, progress)).a,
					texture(_tex3D_BaseTexture, vec3(fragTexCoord.x + delta.x, fragTexCoord.y, progress)).a,
					texture(_tex3D_BaseTexture, vec3(fragTexCoord.x + delta.x, fragTexCoord.y + delta.y, progress)).a,
				};

				minAlpha = alphaSamples[0];
				maxAlpha = alphaSamples[0];
				for (int i = 1; i < 9; i++)
				{
					minAlpha = min(minAlpha, alphaSamples[i]);
					maxAlpha = max(maxAlpha, alphaSamples[i]);
				}
			}

			const vec2 baseTextureSize = textureSize(_tex3D_BaseTexture, 0).xy;

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
		}
	}
	else if (_texMode_BaseTexture == TEXTURE_MODE_2D)
		outColor = texture(_tex2D_BaseTexture, fragTexCoord);
	else
		outColor = vec4(mod(frame.time, 1), 0, 0, 1);

	if (_param_VertexColor)
		outColor *= vec4(rgb, 1);
}