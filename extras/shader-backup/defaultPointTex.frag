#version 450 core

layout(location = 0)out vec4 fragColor;

layout(location = 0)in VS_OUT
{
   vec4 Color;
   vec3 TexCoords;
} fs_in;

layout(binding = 0)uniform sampler2D texture0;

uniform int numPoints;

void main()
{
   vec2 tex = vec2(fs_in.TexCoords.x + gl_PointCoord.x / numPoints, fs_in.TexCoords.y + gl_PointCoord.y / numPoints);
   fragColor = texture(texture0, tex) * fs_in.Color;
}