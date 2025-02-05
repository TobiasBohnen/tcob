R"(#version 100

precision mediump float;
precision mediump sampler2D;

uniform mat4 scissorMat;
uniform mat4 paintMat;

uniform vec2 scissorExt;
uniform vec2 scissorScale;

uniform vec2 extent;
uniform float radius;
uniform float feather;

uniform float strokeMult;
uniform float strokeThr;

uniform int texType;
uniform int type;

uniform vec4 gradientColor;
uniform float gradientIndex;
uniform float gradientAlpha;

uniform sampler2D texture0;
uniform sampler2D gradientTexture;

varying vec2 vertPosition;
varying vec4 vertColor;
varying vec3 vertTexCoords;

const float smoothing = 0.25;

float sdroundrect(vec2 pt, vec2 ext, float rad) {
    vec2 ext2 = ext - vec2(rad, rad);
    vec2 d = abs(pt) - ext2;
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - rad;
}

// Scissoring
float scissorMask(vec2 p) {
    vec2 sc = (abs((scissorMat * vec4(p, 1.0, 1.0)).xy) - scissorExt);
    sc = vec2(0.5, 0.5) - sc * scissorScale;
    return clamp(sc.x, 0.0, 1.0) * clamp(sc.y, 0.0, 1.0);
}

// Stroke - from [0..1] to clipped pyramid, where the slope is 1px.
float strokeMask() {
    return min(1.0, (1.0 - abs(vertTexCoords.s * 2.0 - 1.0)) * strokeMult) * min(1.0, vertTexCoords.t);
}

void main(void) {
    vec4 result;
    float scissor = scissorMask(vertPosition);
    if (scissor == 0.0) discard;
    float strokeAlpha = strokeMask();
    if (strokeAlpha < strokeThr) discard;

    if (type == 0) { // Gradient
        vec4 color;
        if (gradientIndex == -1.0) {
            color = gradientColor;
        } else {
            vec2 pt = (paintMat * vec4(vertPosition, 1.0, 1.0)).xy;
            float idx = clamp((sdroundrect(pt, extent, radius) + feather * 0.5) / feather, 0.0, 1.0);
            color = texture2D(gradientTexture, vec2(idx, gradientIndex));
            color.a *= gradientAlpha;
        }
        // Combine alpha
        color *= strokeAlpha * scissor;
        result = color;
    } else if (type == 1) { // Image
        vec2 pt = (paintMat * vec4(vertPosition, 1.0, 1.0)).xy / extent;
        vec4 color = texture2D(texture0, pt);
        if (texType == 1) {
            // ARGB texture
            color = vec4(color.rgb * color.a, color.a);
        } else if (texType == 2) {
            color = vec4(color.r);
        }
        // Apply color tint and alpha
        color *= gradientColor.rgba;
        // Combine alpha
        color *= strokeAlpha * scissor;
        result = color;
    } else if (type == 2) { // Stencil fill
        result = vec4(1, 1, 1, 1);
    } else if (type == 3) { // Textured tris
        vec4 color = texture2D(texture0, vertTexCoords.xy);
        if (texType == 1) {
            color = vec4(color.rgb * color.a, color.a);
        } else if (texType == 2) {
            color = vec4(color.r);
        }
        // Apply color tint and alpha
        color *= gradientColor.rgba;
        // Combine alpha
        color *= scissor;
        result = color;
    }
    gl_FragColor = result;
}

)"
