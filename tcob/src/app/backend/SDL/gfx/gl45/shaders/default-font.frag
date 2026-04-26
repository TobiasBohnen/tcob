R"(
#version 450 core

layout(location = 0)out vec4 fragColor;

layout(location = 0)in VS_OUT
{
   vec4 color;
   vec3 tex_coords;
} fs_in;

layout(binding = 0)uniform sampler2DArray texture0;

void main() {
   fragColor = fs_in.color * texture(texture0, fs_in.tex_coords).r;
}
)"
