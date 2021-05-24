#version 450 core

layout(location = 0)out vec4 fragColor;

layout(std140, binding = 0)uniform Globals
{
   mat4 camera;
   uvec2 view_size;
   bool debug; 
};

layout(location = 0)in VS_OUT
{
   vec4 Color;
   vec3 TexCoords;
} fs_in;

layout(binding = 0)uniform sampler2D texture0;

void main()
{
   fragColor = fs_in.Color;
}