R"(#version 100
precision mediump float;

uniform vec4 matColor;
uniform float matPointSize;

varying vec4 vertColor;

void main()
{
    gl_FragColor = vertColor * matColor;
}
)"
