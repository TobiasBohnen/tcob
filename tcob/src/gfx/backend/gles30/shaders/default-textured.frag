R"(#version 300 es
precision mediump float;
precision highp sampler2DArray;

uniform Material
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
    fragColor = texture(texture0, vertTexCoords) * vertColor * matColor;
}

)"
