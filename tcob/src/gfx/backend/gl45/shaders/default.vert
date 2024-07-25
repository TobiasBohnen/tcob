R"(
#version 450 core

layout(location = 0)in vec2 vertInPos;
layout(location = 1)in vec4 vertInColor;
layout(location = 2)in vec3 vertInTexCoords;

layout(std140, binding = 0)uniform Globals
{
	mat4 camera;
	uvec2 view_size;
	ivec2 mouse_pos;
	float time;
	bool debug; 
} global;

layout(std140, binding = 1)uniform Material
{
	vec4 color;
	float point_size;
} material;

layout(location = 0)out VS_OUT
{
	vec4 color;
	vec3 tex_coords;
} vs_out;

void main()
{
	vec4 pos = global.camera * vec4(vertInPos.xy, 0.0, 1.0);
	gl_Position = vec4(((pos.x / global.view_size.x) * 2.0 - 1.0), (1.0 - 2.0 * (pos.y / global.view_size.y)), 0.0, 1.0);
	vs_out.color = vertInColor;
	vs_out.tex_coords = vertInTexCoords;
}
)"
