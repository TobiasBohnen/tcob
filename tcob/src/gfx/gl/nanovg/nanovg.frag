R"(
    #version 450 core
    
    layout(std140, binding = 0)uniform frag {
        mat4 scissorMat;
        mat4 paintMat;
        vec4 textOutlineColor;
        vec4 gradient[256];
        vec2 scissorExt;
        vec2 scissorScale;
        vec2 extent;
        float textOutlineThickness;
        float radius;
        float feather;
        float strokeMult;
        float strokeThr;
        int texType;
        int type;
        bool isSingleColor;
    };
    
    uniform sampler2D texture0;
    
    in VS_OUT {
        vec2 Position;
        vec4 Color;
        vec3 TexCoords;
    } fs_in;
    
    out vec4 outColor;
    
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
        return min(1.0, (1.0 - abs(fs_in.TexCoords.s * 2.0 - 1.0)) * strokeMult) * min(1.0, fs_in.TexCoords.t);
    }
    
    void main(void) {
        vec4 result;
        float scissor = scissorMask(fs_in.Position);
        float strokeAlpha = strokeMask();
        if (strokeAlpha < strokeThr)discard;
        if (type == 0) {// Gradient
            // Calculate gradient color using box gradient
            vec4 color;
            if (isSingleColor) {
                color = gradient[0].rgba;
            }
            else {
                vec2 pt = (paintMat * vec4(fs_in.Position, 1.0, 1.0)).xy;
                int idx = int(clamp((sdroundrect(pt, extent, radius) + feather * 0.5) / feather, 0.0, 1.0) * 255);
                color = gradient[idx].rgba;
            }
            // Combine alpha
            color *= strokeAlpha * scissor;
            result = color;
        }else if (type == 1) {// Image
            // Calculate color from texture
            vec2 pt = (paintMat * vec4(fs_in.Position, 1.0, 1.0)).xy / extent;
            vec4 color = texture(texture0, pt);
            if (texType == 1) {
                // ARGB texture
                color = vec4(color.rgb * color.a, color.a);
            }
            if (texType == 2) {
                color = vec4(color.r);
            }
            // Apply color tint and alpha.
            color *= gradient[0].rgba;
            // Combine alpha
            color *= strokeAlpha * scissor;
            result = color;
        }else if (type == 2) {// Stencil fill
            result = vec4(1, 1, 1, 1);
        }else if (type == 3) {// Textured tris
            vec4 color;
            if (texType == 1) {
                color = texture(texture0, fs_in.TexCoords.xy);
                color *= gradient[0].rgba;
            }
            if (texType == 2) {
                // SDF font texture
                float distance = texture(texture0, fs_in.TexCoords.xy).r;
                float outlineFactor = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
                color = mix(textOutlineColor, gradient[0].rgba, outlineFactor);
                float alpha = smoothstep(max(0, textOutlineThickness - smoothing), textOutlineThickness + smoothing , distance);
                color.a *= alpha;
            }
            color *= scissor;
            result = color;
        }
        outColor = result;
    };
)"