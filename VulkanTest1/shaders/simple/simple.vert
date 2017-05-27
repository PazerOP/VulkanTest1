#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "simple_shared.glsl"

#if 0
////////////
// Inputs //
////////////
layout(constant_id = 0) const uint _inputIndex_Position = 0;
layout(location = _inputIndex_Position) in vec2 _input_Position;

layout(constant_id = 1) const uint _inputIndex_Color = 1;
layout(location = _inputIndex_Color) in vec3 _input_Color;

layout(constant_id = 2) const uint _inputIndex_TexCoord = 2;
layout(location = _inputIndex_TexCoord) in vec2 _input_TexCoord;

/////////////
// Outputs //
/////////////
/*layout(constant_id = 3)*/ const uint _outputIndex_Color = 0;
layout(location = _outputIndex_Color) out vec3 _output_Color;

/*layout(constant_id = 4)*/ const uint _outputIndex_TexCoord = 1;
layout(location = _outputIndex_TexCoord) out vec2 _output_TexCoord;
#else
////////////
// Inputs //
////////////
layout(location = 0) in vec2 _input_Position;
layout(location = 1) in vec3 _input_Color;
layout(location = 2) in vec2 _input_TexCoord;

/////////////
// Outputs //
/////////////
layout(location = 0) out vec3 _output_Color;
layout(location = 1) out vec2 _output_TexCoord;
#endif

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = view.orthoProj * view.view * object.modelToWorld * vec4(_input_Position, 0.0, 1.0);
    _output_Color = _input_Color;
	_output_TexCoord = _input_TexCoord;
}