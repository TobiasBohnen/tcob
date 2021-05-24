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
   vec3 tex = vec3(fs_in.TexCoords.x + gl_PointCoord.x, fs_in.TexCoords.y + gl_PointCoord.y, fs_in.TexCoords.z);
   fragColor = texture(texture0, tex) * fs_in.Color;
}