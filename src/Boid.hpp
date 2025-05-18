#pragma once

#include "of3dPrimitives.h"
#include "ofMain.h" // why?
#include "ofxAssimpModel.h"
// #include "Flock.hpp"

class Boid {
public:
    Boid();
    
    void draw(ofx::assimp::Model &model);
    void update();
    glm::vec3 seek(glm::vec3 target);
    void flee(glm::vec3 target);
    void applyForce(glm::vec3 f);
    void setRadii(float sepRadius, float aliRadius, float cohRadius);
    void showRays();
    glm::vec3 separate(const vector<Boid>& boids);
    glm::vec3 align(const vector<Boid>& boids);
    glm::vec3 cohere(const vector<Boid>& boids);
    void applyBehaviors(const vector<Boid>& boids, std::vector<std::vector<float>>& heightMap);
    bool checkUnderHeightMap(glm::vec3 pos, std::vector<std::vector<float>>& heightMap);
    void checkEdges();
    vector<glm::vec3> getRays() const;

    float collisionRadius = 0.25f;

    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    float maxSpeed = 0.1;
    float maxForce = 0.005;
    ofColor fishColor;
    bool underHeight = false;
    ofColor oldColor;
private:
    float separationRadius = 8.0; // Default separation radius
    float alignmentRadius = 15.0; // Default alignment radius
    float cohesionRadius = 15.0; // Default cohesion radius
};