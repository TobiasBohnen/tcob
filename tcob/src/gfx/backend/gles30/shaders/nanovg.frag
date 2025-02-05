R"(#version 300 es
precision mediump float;
precision mediump sampler2DArray;

layout(std140)uniform Ubo {
    mat4 scissorMat;
    mat4 paintMat;

    vec2 scissorExt;
    vec2 scissorScale;

    vec2 extent;
    float radius;
    float feather;

    float strokeMult;
    float strokeThr;

    int texType;
    int type;

    vec4 gradientColor;
    float gradientIndex;
    float gradientAlpha;
} frag;

uniform sampler2DArray texture0;
uniform sampler2DArray gradientTexture;

in vec2 vertPosition;
in vec4 vertColor;
in vec3 vertTexCoords;

out vec4 outColor;

const float smoothing = 0.25;

float sdroundrect(vec2 pt, vec2 ext, float rad) {
    vec2 ext2 = ext - vec2(rad, rad);
    vec2 d = abs(pt) - ext2;
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - rad;
}

// Scissoring
float scissorMask(vec2 p) {
    vec2 sc = (abs((frag.scissorMat * vec4(p, 1.0, 1.0)).xy) - frag.scissorExt);
    sc = vec2(0.5, 0.5) - sc * frag.scissorScale;
    return clamp(sc.x, 0.0, 1.0) * clamp(sc.y, 0.0, 1.0);
}

// Stroke - from [0..1] to clipped pyramid, where the slope is 1px.
float strokeMask() {
    return min(1.0, (1.0 - abs(vertTexCoords.s * 2.0 - 1.0)) * frag.strokeMult) * min(1.0, vertTexCoords.t);
}

void main(void) {
    vec4 result;
    float scissor = scissorMask(vertPosition);
    if (scissor == 0.0)discard;
    float strokeAlpha = strokeMask();
    if (strokeAlpha < frag.strokeThr)discard;

    if (frag.type == 0) {// Gradient
        // Calculate gradient color using box gradient
        vec4 color;
        if (frag.gradientIndex == -1.0) {
            color = frag.gradientColor;
        }else {
            vec2 pt = (frag.paintMat * vec4(vertPosition, 1.0, 1.0)).xy;
            float idx = clamp((sdroundrect(pt, frag.extent, frag.radius) + frag.feather * 0.5) / frag.feather, 0.0, 1.0);
            color = texture(gradientTexture, vec3(idx, frag.gradientIndex, 0));
            color.a *= frag.gradientAlpha;
        }
        // Combine alpha
        color *= strokeAlpha * scissor;
        result = color;
    }else if (frag.type == 1) {// Image
        // Calculate color from texture
        vec2 pt = (frag.paintMat * vec4(vertPosition, 1.0, 1.0)).xy / frag.extent;
        vec4 color = texture(texture0, vec3(pt, 0));
        if (frag.texType == 1) {
            // ARGB texture
            color = vec4(color.rgb * color.a, color.a);
        }else if (frag.texType == 2) {
            color = vec4(color.r);
        }
        // Apply color tint and alpha.
        color *= frag.gradientColor.rgba;
        // Combine alpha
        color *= strokeAlpha * scissor;
        result = color;
    }else if (frag.type == 2) {// Stencil fill
        result = vec4(1, 1, 1, 1);
    }else if (frag.type == 3) {// Textured tris
        vec4 color = texture(texture0, vertTexCoords);
        if (frag.texType == 1) {
            color = vec4(color.rgb * color.a, color.a);
        }else if (frag.texType == 2) {
            color = vec4(color.r);
        }
        // Apply color tint and alpha.
        color *= frag.gradientColor.rgba;
        // Combine alpha
        color *= scissor;
        result = color;
    }
    outColor = result;
}
)"
