R"(
#version 450 core

layout(std140, binding = 1)uniform Material
{
	vec4 color;
	float point_size;
} material;

layout(location = 0)out vec4 fragColor;

layout(location = 0)in VS_OUT
{
   vec4 color;
   vec3 tex_coords;
} fs_in;

void main()
{
   fragColor = fs_in.color * material.color;
}
)"
