R"(
#version 450 core

layout(location = 0)out vec4 fragColor;

layout(location = 0)in VS_OUT
{
   vec4 color;
   vec3 tex_coords;
} fs_in;

layout(binding = 0)uniform sampler2DArray texture0;

const float smoothing = 1.0 / 16.0;

void main() {
   float distance = texture(texture0, fs_in.tex_coords).r;
   vec4 color;
      if (distance >= 0.5) {
         float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
            
         color.rgb = fs_in.color.rgb;
         color.a = fs_in.color.a * alpha;
      }

   fragColor = color;
}
)"
