
OF_GLSL_SHADER_HEADER

out vec4 outputColor;

// uniform sampler2D
uniform sampler2D grassTexture;
uniform sampler2D rockTexture;
uniform vec4 globalColor;
uniform float time;
uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

in vec3 FragPos;
in vec3 Normal;
in vec4 fragPosLightSpace;
in vec2 Texcoord;
float near = 0.1;
float far = 100.0;

// float shadowCalculation(vec4 fragPosLightSpace) {
//   vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
//   projCoords = projCoords * 0.5 + 0.5;

//   float closestDepth = texture(shadowMap, projCoords.xy).r;
//   float currentDepth = projCoords.z;
//   float shadow = currentDepth > closestDepth + 0.005
//                      ? 1.0
//                      : 0.0; // Add bias to prevent shadow acne
//   return shadow;
//   // return closestDepth;
// }

void main() {
  vec3 Color = vec3(0.431, 0.118, 0.024);
  vec4 trasparent_water = vec4(0.7, 0.7, 0.7, 0.3);
  vec2 uv = gl_FragCoord.xy / u_resolution.xy;
  float height = FragPos.y;
  vec3 normal = normalize(Normal);

  vec4 grassColor = texture2D(grassTexture, Texcoord);
  vec4 rockColor = texture2D(rockTexture, Texcoord);

  float steepness =
      1.0 - dot(normal,
                vec3(0., 1., 0.)); // dot the normal with up, 0 means very
                                   // steep, and 1 means flat, we want to invert
                                   // that, and therefore subtract it from 1
  if (height < 0) {
    Color = vec3(0.0, 0.3, 0.6); // low (possibly water)
    // Color = grassColor.xyz; // grass

  } else if (height < 0.5) {
    Color = grassColor.xyz; // grass
  } else if (height > 0.5) {
    // Color = vec3(0.8, 0.3, 0.3); // rocky
    // Color = grassColor.xyz; // grass
    Color = rockColor.xyz; // rocky
  } else if (steepness > 0.6) {
    Color = vec3(0.6, 0.3, 0.3); // rocky
  }
  // ambient calculations
  float ambientStrength = 0.8;
  vec3 ambient = ambientStrength * normalize(lightColor);
  // diffuse calculations
  // vec3 normal = (vec3(Normal.x, Normal.z, Normal.y) + 1.0) * 0.5;
  vec3 lightDir = normalize(lightPos - FragPos);
  float diff = max(dot(normal, lightDir), 0.0); // prevent from going negative
  vec3 diffuse = diff * lightColor;
  // specular
  float specularStrength = 0.2;
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
  vec3 specular = specularStrength * spec * lightColor;
  // Combine results
  // float shadow = shadowCalculation(fragPosLightSpace);
  // vec3 result = ambient + (1 - shadow) * (diffuse + specular);
  vec3 result = ambient + diffuse + specular;
  outputColor = vec4(0., 0., 0., 0.);
  if (normal.y == 1) {

    outputColor = trasparent_water;

  } else {
    // result = result * normalize(Color);
    outputColor = vec4(Color, 1.0);
  }
  // outputColor = texture2D(grassTexture, uv);
  // outputColor = vec4(Texcoord, 0.0, 1.0);
}