R"(#version 100
precision mediump float;
precision highp sampler2DArray;

uniform vec4 matColor;
uniform float matPointSize;

varying vec4 vertColor;
varying vec3 vertTexCoords;

uniform sampler2D texture0;

void main()
{
    vec4 textureColor = texture2D(texture0, vertTexCoords.xy);
    gl_FragColor = textureColor * vertColor * matColor;
}
)"
