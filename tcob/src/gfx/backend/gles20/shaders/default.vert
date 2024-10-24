R"(#version 100

precision mediump float;

attribute vec2 vertInPos;
attribute vec4 vertInColor;
attribute vec3 vertInTexCoords;

uniform mat4 camera;
uniform ivec2 view_size;
uniform ivec2 mouse_pos;
uniform float time;
uniform bool debug; 

uniform vec4 matColor;
uniform float matPointSize;

varying vec4 vertColor;
varying vec3 vertTexCoords;

void main()
{
    gl_PointSize = matPointSize;
    vec4 pos = camera * vec4(vertInPos.xy, 0.0, 1.0);
    gl_Position = vec4(((pos.x / float(view_size.x)) * 2.0 - 1.0), (1.0 - 2.0 * (pos.y / float(view_size.y))), 0.0, 1.0);
    vertColor = vertInColor;
    vertTexCoords = vertInTexCoords;
}
)"
