#version 450 core

layout(location = 0)out vec4 fragColor;

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

const float smoothing = 1.0 / 16.0;

void main() {
   float distance = texture(texture0, fs_in.TexCoords.xy).r;
   vec4 color;

   if (outline_thickness < 0.5)
   {
      vec4 oc = outline_color;
      oc.a = fs_in.Color.a;
      
      float outlineFactor = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
      color = mix(outline_color, fs_in.Color, outlineFactor);
      float alpha = smoothstep(max(0, outline_thickness - smoothing), outline_thickness + smoothing, distance);
      color.a *= alpha;
      /*
      if (distance >= 0 && distance <= outline_thickness ) {
         color.rgb = fs_in.Color.rgb;
         color.a = 0.0;
      }
      else if (distance >= 0.5) {
         color.rgb = fs_in.Color.rgb;
         color.a = fs_in.Color.a;
      }
      else {
         float alpha = smoothstep(max(outline_thickness - smoothing, 0), outline_thickness + smoothing, distance);

         color.rgb = outline_color.rgb;
         color.a = fs_in.Color.a * alpha;
      }
      */
   }
   else
   {
      if (distance >= 0.5) {
         float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
            
         color.rgb = fs_in.Color.rgb;
         color.a = fs_in.Color.a * alpha;
      }
   }

   fragColor = color;
}