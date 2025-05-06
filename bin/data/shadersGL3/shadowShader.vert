#version 330 core

uniform mat4 modelViewProjectionMatrix;
uniform mat4 lightSpaceMatrix;
uniform mat4 model;

in vec4 position;


void main() {
  gl_Position = lightSpaceMatrix * model * position;
}