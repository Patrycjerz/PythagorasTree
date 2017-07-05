#version 150

uniform mat4 mvp_matrix;

in vec4 in_position;
in vec4 in_normal;

out vec4 inout_normal;

void main()
{
	gl_Position = mvp_matrix * in_position;
	inout_normal = in_normal;
}