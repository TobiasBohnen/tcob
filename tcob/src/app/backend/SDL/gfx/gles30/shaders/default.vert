R"(#version 300 es
precision mediump float;

in vec2 vertInPos;
in vec4 vertInColor;
in vec3 vertInTexCoords;

uniform Globals
{
    mat4 camera;
    uvec2 view_size;
    ivec2 mouse_pos;
    float time;
    bool debug; 
};

uniform Material
{
   vec4 matColor;
   float matPointSize;
};

out vec4 vertColor;
out vec3 vertTexCoords;

void main()
{
    gl_PointSize = matPointSize;
    vec4 pos = camera * vec4(vertInPos.xy, 0.0, 1.0);
    gl_Position = vec4(((pos.x / float(view_size.x)) * 2.0 - 1.0), (1.0 - 2.0 * (pos.y / float(view_size.y))), 0.0, 1.0);
    vertColor = vertInColor;
    vertTexCoords = vertInTexCoords;
}
)"
