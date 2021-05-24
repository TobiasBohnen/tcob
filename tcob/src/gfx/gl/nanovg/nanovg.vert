R"(		
#version 450 core

uniform vec2 viewSize; 

layout(location = 0) in vec2 vertPos;
layout(location = 1) in vec4 vertColor;
layout(location = 2) in vec3 vertTexCoords;

out VS_OUT {
   vec2 Position;
   vec4 Color;
   vec3 TexCoords;
} vs_out;

void main(void) { 
    gl_Position = vec4(2.0 * vertPos.x / viewSize.x - 1.0, 1.0 - 2.0 * vertPos.y / viewSize.y, 0, 1);
    vs_out.Position = vertPos;
    vs_out.Color = vertColor;
    vs_out.TexCoords = vertTexCoords;
}
)"