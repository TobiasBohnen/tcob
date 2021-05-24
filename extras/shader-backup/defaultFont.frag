#version 450 core

layout(location = 0)out vec4 fragColor;

layout(std140, binding = 0)uniform Globals
{
   mat4 camera;
   uvec2 view_size;
   bool debug; 
};

layout(std140, binding = 1)uniform Text
{
   vec4 outline_color;
   float outline_thickness;
};

layout(location = 0)in VS_OUT
{
   vec4 Color;
   vec3 TexCoords;
} fs_in;

layout(binding = 0)uniform sampler2D texture0;

const float smoothing = 1.0 / 32.0;

void main() {
   if (debug) {
      fragColor = fs_in.Color;
   } else {
      float distance = texture(texture0, fs_in.TexCoords.xy).r;
      
      float outlineFactor = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
      vec4 color = mix(outline_color, fs_in.Color, outlineFactor);
      float alpha = smoothstep(max(0, outline_thickness - smoothing), outline_thickness + smoothing, distance);
      
      fragColor = vec4(color.rgb, color.a * alpha);
   }
}