R"(#version 100

precision mediump float;

uniform vec2 viewSize;

attribute vec2 vertInPos;
attribute vec4 vertInColor;
attribute vec3 vertInTexCoords;

varying vec2 vertPosition;
varying vec4 vertColor;
varying vec3 vertTexCoords;

void main(void) {
    gl_Position = vec4(2.0 * vertInPos.x / viewSize.x - 1.0, 1.0 - 2.0 * vertInPos.y / viewSize.y, 0.0, 1.0);
    vertPosition = vertInPos;
    vertColor = vertInColor;
    vertTexCoords = vertInTexCoords;
}

)"
