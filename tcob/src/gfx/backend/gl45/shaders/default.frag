R"(
#version 450 core

layout(location = 0)out vec4 fragColor;

layout(location = 0)in VS_OUT
{
   vec4 Color;
   vec3 TexCoords;
} fs_in;

void main()
{
   fragColor = fs_in.Color;
}
)"
