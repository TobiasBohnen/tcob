#version 450 core

layout(location = 0)in vec2 vertPos;
layout(location = 1)in vec4 vertColor;
layout(location = 2)in vec3 vertTexCoords;

layout(std140, binding = 0)uniform Globals
{
   mat4 camera;
   uvec2 view_size;
   bool debug; 
};

layout(location = 0)out VS_OUT
{
	vec4 Color;
	vec3 TexCoords;
} vs_out;

void main()
{
	gl_Position = camera * vec4(vertPos.x * 2.0 - 1.0, 1.0 - 2.0 * vertPos.y, 0.0, 1.0);
	vs_out.Color = vertColor;
	vs_out.TexCoords = vertTexCoords;
}