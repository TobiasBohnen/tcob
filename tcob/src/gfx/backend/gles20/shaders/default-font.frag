R"(#version 100
precision mediump float;
precision highp sampler2DArray;

varying vec4 vertColor;
varying vec3 vertTexCoords;

uniform sampler2D texture0;

const float smoothing = 1.0 / 16.0;

void main() {
    float distance = texture2D(texture0, vertTexCoords.xy).r;
    vec4 color;

    if (distance >= 0.5) {
        float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
        
        color.rgb = vertColor.rgb;
        color.a = vertColor.a * alpha;
    } else {
        color = vec4(0.0); // Default color if distance < 0.5
    }

    gl_FragColor = color;
}
)"
