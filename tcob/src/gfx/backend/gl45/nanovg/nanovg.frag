R"(
#version 450 core

layout(std140, binding = 0)uniform Ubo {
    mat4 ScissorMatrix;
    mat4 PaintMatrix;
    vec4 Gradient[256];
    vec2 ScissorExt;
    vec2 ScissorScale;
    vec2 Extent;
    float Radius;
    float Feather;
    float StrokeMult;
    float StrokeThr;
    int TexType;
    int Type;
    bool IsSingleColor;
} frag;

uniform sampler2DArray texture0;

in VS_OUT {
    vec2 Position;
    vec4 Color;
    vec3 TexCoords;
} fs_in;

out vec4 outColor;

const float smoothing = 0.25;

float sd_round_rect(vec2 pt, vec2 ext, float rad) {
    vec2 ext2 = ext - vec2(rad, rad);
    vec2 d = abs(pt) - ext2;
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - rad;
}

// Scissoring
float scissor_mask(vec2 p) {
    vec2 sc = (abs((frag.ScissorMatrix * vec4(p, 1.0, 1.0)).xy) - frag.ScissorExt);
    sc = vec2(0.5, 0.5) - sc * frag.ScissorScale;
    return clamp(sc.x, 0.0, 1.0) * clamp(sc.y, 0.0, 1.0);
}

// Stroke - from [0..1] to clipped pyramid, where the slope is 1px.
float stroke_mask() {
    return min(1.0, (1.0 - abs(fs_in.TexCoords.s * 2.0 - 1.0)) * frag.StrokeMult) * min(1.0, fs_in.TexCoords.t);
}

void main(void) {
    vec4 result;
    float scissor = scissor_mask(fs_in.Position);
    if (scissor == 0)discard;
    float strokeAlpha = stroke_mask();
    if (strokeAlpha < frag.StrokeThr)discard;

    if (frag.Type == 0) {// Gradient
        // Calculate gradient color using box gradient
        vec4 color;
        if (frag.IsSingleColor) {
            color = frag.Gradient[0].rgba;
        } else {
            vec2 pt = (frag.PaintMatrix * vec4(fs_in.Position, 1.0, 1.0)).xy;
            int idx = int(clamp((sd_round_rect(pt, frag.Extent, frag.Radius) + frag.Feather * 0.5) / frag.Feather, 0.0, 1.0) * 255.0);
            color = frag.Gradient[idx].rgba;
        }
        // Combine alpha
        color *= strokeAlpha * scissor;
        result = color;
    }else if (frag.Type == 1) {// Image
        // Calculate color from texture
        vec2 pt = (frag.PaintMatrix * vec4(fs_in.Position, 1.0, 1.0)).xy / frag.Extent;
        vec4 color = texture(texture0, vec3(pt, 0));
        if (frag.TexType == 1) {
            // ARGB texture
            color = vec4(color.rgb * color.a, color.a);
        }else if (frag.TexType == 2) {
            color = vec4(color.r);
        }
        // Apply color tint and alpha.
        color *= frag.Gradient[0].rgba;
        // Combine alpha
        color *= strokeAlpha * scissor;
        result = color;
    }else if (frag.Type == 2) {// Stencil fill
        result = vec4(1, 1, 1, 1);
    }else if (frag.Type == 3) {// Textured tris
        vec4 color = texture(texture0, fs_in.TexCoords);
        if (frag.TexType == 1) {
            color = vec4(color.rgb * color.a, color.a);
        }else if (frag.TexType == 2) {
            color = vec4(color.r);
        }
        // Apply color tint and alpha.
        color *= frag.Gradient[0].rgba;
        // Combine alpha
        color *= scissor;
        result = color;
    }
    outColor = result;
}
)"
