R"(
#version 450 core

layout(location = 0)out vec4 fragColor;

layout(location = 0)in VS_OUT
{
   vec4 Color;
   vec3 TexCoords;
} fs_in;

layout(binding = 0)uniform sampler2DArray texture0;

void main()
{
   fragColor = texture(texture0, fs_in.TexCoords) * fs_in.Color;
}
)"
