R"(#version 300 es
precision mediump float;
precision highp sampler2DArray;

uniform Pass
{
    vec4 matColor;
    float matPointSize;
};

out vec4 fragColor;

in vec4 vertColor;
in vec3 vertTexCoords;

uniform sampler2DArray texture0;

void main() 
{
   fragColor = vec4(vertColor.rgb, vertColor.a * texture(texture0, vertTexCoords).r);
}

)"
