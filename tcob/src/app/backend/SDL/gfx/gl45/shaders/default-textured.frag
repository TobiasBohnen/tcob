R"(
#version 450 core

layout(std140, binding = 1)uniform Pass
{
	vec4 color;
} pass;

layout(location = 0)out vec4 fragColor;

layout(location = 0)in VS_OUT
{
   vec4 color;
   vec3 tex_coords;
} fs_in;

layout(binding = 0)uniform sampler2DArray texture0;

void main()
{
   fragColor = texture(texture0, fs_in.tex_coords) * fs_in.color * pass.color;
}
)"
