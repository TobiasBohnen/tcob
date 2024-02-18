R"(	
#version 300 es
precision mediump float;

uniform vec2 viewSize;

in vec2 vertInPos;
in vec4 vertInColor;
in vec3 vertInTexCoords;

out vec2 vertPosition;
out vec4 vertColor;
out vec3 vertTexCoords;

void main(void) {
    gl_Position = vec4(2.0 * vertInPos.x / viewSize.x - 1.0, 1.0 - 2.0 * vertInPos.y / viewSize.y, 0, 1);
    vertPosition = vertInPos;
    vertColor = vertInColor;
    vertTexCoords = vertInTexCoords;
}

)"
