#version 440

struct Particle {
  vec4 pos;
  vec4 vel;
  vec4 col;
};

layout(std140, binding = 0) buffer particle { Particle p[]; };
layout(std140, binding = 1) buffer particlesBack { Particle p2[]; };
layout(local_size_x = 512, local_size_y = 1, local_size_z = 1) in;

uniform float timeLastFrame;
uniform float emitterX;
uniform float emitterY;
uniform float emitterZ;
uniform float emitterR;

float rand(float min, float max, float seed) {
  return min +
         (max - min) * fract(sin(seed * 12.9898 + 78.233) *
                             43758.5453);
}
void main() {
  uint id = gl_GlobalInvocationID.x;
  vec4 acceleration = vec4(
      0, -8, 0, 0); // use acceleration to intergrate velocity and position
                    // Update the position based on velocity and timeLastFrame
  float dt = 0.016;
  p[id].vel.xyz = p2[id].vel.xyz + acceleration.xyz * dt;

  p[id].pos.xyz = p2[id].pos.xyz + (p[id].vel.xyz + p2[id].vel.xyz) * (dt / 2);

  // // Optional: Add boundary checks to keep particles within a range
  // if (p[id].pos.x < -500.0 || p[id].pos.x > 500.0) p[id].vel.x *= -1.0;
  // if (p[id].pos.y < -500.0 || p[id].pos.y > 500.0) p[id].vel.y *= -1.0;
  // if (p[id].pos.z < -500.0 || p[id].pos.z > 500.0) p[id].vel.z *= -1.0;
  // p[id].pos.x = 50.0;
  p[id].pos.w = p2[id].pos.w - dt;
  if (p[id].pos.w < 0) {
    p[id].pos.w = rand(1, 3, gl_GlobalInvocationID.x * 19.0);
    p[id].vel.x = rand(-15, 15, gl_GlobalInvocationID.x * 7.0);
    p[id].vel.y = 10;
    p[id].vel.z = rand(-15, 15, gl_GlobalInvocationID.x * 11.0);
		// p[id].pos.xyz = vec3(emitterX + emitterR, emitterY,emitterZ);
		p[id].pos.x = rand(emitterX - emitterR, emitterX + emitterR, gl_GlobalInvocationID.x * 23);
		p[id].pos.y = rand(emitterY, emitterY, gl_GlobalInvocationID.x * 29);
		p[id].pos.z = rand(emitterZ - emitterR, emitterZ + emitterR, gl_GlobalInvocationID.x * 31);

	}
  // p[id].pos.xyz += p[id].vel.xyz * timeLastFrame;

  p[id].col = vec4(1.0, // Red
                   1.0, // Green
                   0.,  // Blue
                   1.0  // Alpha
  );
}