#version 330 core

// OF_GLSL_SHADER_HEADER
// layout(location = 0) in vec3 aPos;
// layout(location = 1) in vec3 aNormal;
uniform mat4 modelViewProjectionMatrix;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform sampler2D grassTexture;

uniform mat4 customMVPMatrix;
in vec4 position;
in vec2 texcoord;
in vec3 color_coord;
in vec3 normal;
uniform float time;
uniform mat4 lightSpaceMatrix;

out vec3 FragPos;
out vec3 Normal;
out vec3 Color;
out vec2 Texcoord;
out vec4 fragPosLightSpace;

void main() {
  // this makes it so that we are now in screen space
  gl_Position = customMVPMatrix * model * position;
  // Fragpos gives us the world space of the model
  FragPos = vec3(model * position);
  vec4 wp = model * position;
  fragPosLightSpace = lightSpaceMatrix * wp; // Transform to light's clip space

  Normal = mat3(transpose(inverse(model))) * normal;
  // Normal = normal;
  Color = color_coord;
  Texcoord = texcoord;
}