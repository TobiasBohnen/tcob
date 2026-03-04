R"(
#version 450 core

layout(std140, binding = 1)uniform Pass
{
	vec4 color;
	float point_size;
} pass;

layout(location = 0)out vec4 fragColor;

layout(location = 0)in VS_OUT
{
   vec4 color;
   vec3 tex_coords;
} fs_in;

void main()
{
   fragColor = fs_in.color * pass.color;
}
)"
