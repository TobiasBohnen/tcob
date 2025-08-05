R"(#version 300 es
precision mediump float;
precision highp sampler2DArray;

out vec4 fragColor;

in vec4 vertColor;
in vec3 vertTexCoords;

uniform sampler2DArray texture0;

const float smoothing = 1.0 / 16.0;

void main() {
    float distance = texture(texture0, vertTexCoords).r;
    vec4 color;

    if (distance >= 0.5) {
        float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
        
        color.rgb = vertColor.rgb;
        color.a = vertColor.a * alpha;
    }

    fragColor = color;
}

)"
