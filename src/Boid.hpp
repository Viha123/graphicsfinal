#pragma once

#include "of3dPrimitives.h"
#include "ofMain.h" // why?
#include "ofxAssimpModel.h"
// #include "Flock.hpp"

class Boid {
public:
  Boid();
  struct BoidParams {
    float preyMaxSpeed;
    float preyMaxForce;
    float predatorMaxSpeed;
    float predatorMaxForce;
    float predatorVisionRadius;
    float preyVisionRadius;
    float interactionRadius;
    float separationRadius;
    float alignmentRadius;
    float cohesionRadius;
  };
  struct Features {
    bool enableCollisionRays;
    bool enableSeekFoodPoint;
    bool showMeshCollision;
    bool showHealth;
  };
  void draw(ofx::assimp::Model &model);
  void update();
  glm::vec3 seek(glm::vec3 target);
  glm::vec3 flee(glm::vec3 target);
  void applyForce(glm::vec3 f);
  void setRadii(float sepRadius, float aliRadius, float cohRadius);
  void showRays();
  void showSeek();
  
  void updateParams(const BoidParams &params, const Features &features);
  glm::vec3 separate(const vector<Boid> &boids);
  glm::vec3 align(const vector<Boid> &boids);
  glm::vec3 cohere(const vector<Boid> &boids);
  glm::vec3 fleeCollision(std::vector<std::vector<float>> &heightMap);
  void applyBehaviors(const vector<Boid> &boids, const vector<Boid> &predators,
                      const vector<Boid> &prey,
                      std::vector<std::vector<float>> &heightMap);
  bool checkUnderHeightMap(glm::vec3 pos,
                           std::vector<std::vector<float>> &heightMap);
  void checkEdges();
  vector<glm::vec3> getRays() const;
  void checkInteraction(vector<Boid> &predators);

  float collisionRadius = 15.0f; // how far the rays are cast

  glm::vec3 position;
  glm::vec3 velocity;
  glm::vec3 acceleration;
  glm::vec3 seekPosition;
  float maxSpeed = 0.1;
  float maxForce = 0.005;
  ofColor fishColor;
  bool underHeight = false;
  ofColor oldColor;
  std::string type = "prey";

  bool toggleShowRays = false;
  bool toggleShowSeek = false;
  bool toggleShowMeshCollision = false;
  bool toggleHealth = false;

  float visionRadius =
      30.0; // vision for prety and predator are different. predator uses it to
            // detect prey, and prey uses it to detect predators and find food
  float interactionRadius =
      5.0; // same for everyone, detect if 2 things tougched

  int health = 10000;
  int maxHealth = 10000;

  float separationRadius = 20.0; // Default separation radius
  float alignmentRadius = 35.0;  // Default alignment radius
  float cohesionRadius = 35.0;   // Default cohesion radius
};