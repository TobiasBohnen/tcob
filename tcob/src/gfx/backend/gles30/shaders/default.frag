R"(
#version 300 es
precision mediump float;

out vec4 fragColor;

in vec4 vertColor;
in vec3 vertTexCoords;

void main()
{
   fragColor = vertColor;
}
)"
