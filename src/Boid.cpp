#include "Boid.hpp"
#include "ofColor.h"
#include "ofGraphics.h"
#include "quaternion.hpp"
#include <limits>

glm::mat4 rotateToVector(glm::vec3 v1, glm::vec3 v2) {
  glm::vec3 axis = glm::cross(v1, v2);
  glm::quat q = glm::angleAxis(glm::angle(v1, v2), glm::normalize(axis));
  return glm::toMat4(q);
}

glm::vec3 rotateVectorRad(const glm::vec3 &vec, float angleRad,
                          const glm::vec3 &axis) {
  glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angleRad, axis);
  glm::vec4 rotatedVec = rotationMatrix * glm::vec4(vec, 1.0f);
  return glm::vec3(rotatedVec);
}

vector<glm::vec3> Boid::getRays() const {
  vector<glm::vec3> rays;

  glm::vec3 forward = normalize(velocity) * collisionRadius;
  rays.push_back(forward);

  // Construct a coordinate basis around the forward vector
  // Pick an arbitrary "up" vector that's not parallel to velocity
  glm::vec3 up = abs(dot(velocity, glm::vec3(0, 1, 0))) < 0.99
                     ? glm::vec3(0, 1, 0)
                     : glm::vec3(1, 0, 0);

  glm::vec3 right = normalize(glm::cross(velocity, up));
  glm::vec3 adjustedUp = normalize(glm::cross(velocity, right));

  // Horizontal 45° rays (left and right)
  float angle45 = PI / 4.0f;

  glm::vec3 left45 =
      normalize(rotateVectorRad(velocity, +angle45, adjustedUp)) *
      collisionRadius;
  glm::vec3 right45 =
      normalize(rotateVectorRad(velocity, -angle45, adjustedUp)) *
      collisionRadius;
  rays.push_back(left45);
  rays.push_back(right45);

  // Vertical 45° rays (up and down)
  glm::vec3 up45 =
      normalize(rotateVectorRad(velocity, -angle45, right)) * collisionRadius;
  glm::vec3 down45 =
      normalize(rotateVectorRad(velocity, +angle45, right)) * collisionRadius;
  rays.push_back(up45);
  rays.push_back(down45);

  return rays;
}

void Boid::showRays() {
  vector<glm::vec3> collisionRays = getRays();
  for (auto ray : collisionRays) {
    glm::vec3 end = position + ray;
    ofDrawLine(position.x, position.y, position.z, end.x, end.y, end.z);
  }
}

Boid::Boid() {
  velocity =
      glm::vec3(ofRandom(-0.1, 0.1), ofRandom(0, 0), ofRandom(-0.1, 0.1));
  acceleration =
      glm::vec3(ofRandom(-0.1, 0.1), ofRandom(-0.1, 0.1), ofRandom(-0.1, 0.1));
  position =
      glm::vec3(ofRandom(-300, 300), ofRandom(-50, 0), ofRandom(-300, 300));

  if (ofRandom(1) < 0.5) {
    // Generate a random shade of orange
    fishColor.setHsb(ofRandom(20, 40), ofRandom(150, 255), ofRandom(150, 255));
  } else {
    // Generate a random shade of blue
    fishColor.setHsb(ofRandom(190, 210), ofRandom(150, 255),
                     ofRandom(150, 255));
  }

  oldColor = fishColor;
}

void Boid::draw(ofx::assimp::Model &model) {
  model.enableColors();

  ofSetColor(fishColor);
  if (type == "food") {
    ofDrawSphere(position, 1);
    return;
  }
  // cout << fishColor << endl;
  if (glm::length(velocity) > 0) {
    glm::vec3 dir = glm::normalize(velocity);
    glm::vec3 coneDir = glm::vec3(0, 0, -1);

    glm::mat4 rotationMatrix = rotateToVector(coneDir, dir);
    glm::mat4 coneTransform =
        glm::translate(glm::mat4(1.0f), position) * rotationMatrix;
    // coneTransform = glm::scale(coneTransform, glm::vec3(0.1, 0.1, 0.1));
    if (type == "predator") {
      coneTransform = glm::scale(coneTransform, glm::vec3(2, 2, 2));
    }
    
    ofPushMatrix();
    ofMultMatrix(coneTransform);
    model.draw();
    // ofDrawCone(0, 0, 0, 0.3, 1.0);
    ofPopMatrix();

    // drawing rays for each boid
    if (toggleShowRays) {
      showRays();
    }

    if (toggleShowSeek) {
      showSeek();
    }
    if (toggleHealth) {
      ofSetColor(fishColor);
      ofDrawBitmapString(std::to_string(health), position.x, position.y + 10,
                     position.z);
    }

    // set the seek pos
    if (type == "predator") {
      ofSetColor(ofColor::green);
      ofDrawSphere(seekPosition, 1);
    }

  } else {
    ofPushMatrix();
    ofTranslate(position);
    // model.draw();
    ofDrawCone(0, 0, 0, 0.3, 1.0);
    ofPopMatrix();
  }
}

void Boid::update() {
  velocity += acceleration;
  if (glm::length(velocity) > maxSpeed) {
    velocity = glm::normalize(velocity) * maxSpeed;
  }
  position += velocity;
  // bounding position
  checkEdges();

  acceleration *= 0;
}

void Boid::setRadii(float sepRadius, float aliRadius, float cohRadius) {
  separationRadius = sepRadius;
  alignmentRadius = aliRadius;
  cohesionRadius = cohRadius;
}

void Boid::applyForce(glm::vec3 force) { acceleration += force; }

glm::vec3 Boid::seek(glm::vec3 target) {
  glm::vec3 desired = target - position;
  glm::vec3 steer = glm::normalize(desired) * maxSpeed;
  if (glm::length(steer) > maxForce) {
    steer = glm::normalize(steer) * maxForce;
  }
  return steer;
}

glm::vec3 Boid::flee(glm::vec3 target) {
  glm::vec3 desired = -(target - position);
  glm::vec3 steer = glm::normalize(desired) * 0.05;
  if (glm::length(steer) > maxForce) {
    steer = glm::normalize(steer) * maxForce;
  }
  // applyForce(steer);
  return steer;
}

// Passing in a const reference to ensure correct comparison of boid objects
glm::vec3 Boid::separate(const vector<Boid> &boids) {
  float desiredSeparation = separationRadius;
  glm::vec3 sum = glm::vec3(0, 0, 0);
  int count = 0;
  for (auto &other : boids) {
    float dist = glm::distance(position, other.position);
    // compares memory addresses to check if they're the same object
    if (&other != this && dist < desiredSeparation) {
      glm::vec3 diff = position - other.position;
      // the closer the other is, the faster you flee
      diff = glm::normalize(diff) * (1 / dist);
      sum += diff;
      count++;
    }
  }

  if (count > 0) {
    sum = glm::normalize(sum) * maxSpeed;
    glm::vec3 steer = sum - velocity;
    if (glm::length(steer) > maxForce) {
      steer = glm::normalize(steer) * maxForce;
    }
    return steer;
  }
  return glm::vec3(0, 0, 0);
}

// TODO: only have line of sight of boids in a cone in front
glm::vec3 Boid::align(const vector<Boid> &boids) {
  float neighborDistance = alignmentRadius;
  int count = 0;
  glm::vec3 sum = glm::vec3(0, 0, 0);
  for (auto &other : boids) {
    float dist = glm::distance(position, other.position);
    if (&other != this && dist < neighborDistance) {
      sum += other.velocity;
      count++;
    }
  }
  if (count > 0) {
    // sum /= boids.size();
    sum = glm::normalize(sum) * maxSpeed;
    glm::vec3 steer = sum - velocity;
    if (glm::length(steer) > maxForce) {
      steer = glm::normalize(steer) * maxForce;
    }
    return steer;
  }
  return glm::vec3(0, 0, 0);
}

glm::vec3 Boid::cohere(const vector<Boid> &boids) {
  float neighborDistance = cohesionRadius;
  int count = 0;
  glm::vec3 sum = glm::vec3(0, 0, 0);
  for (auto &other : boids) {
    float dist = glm::distance(position, other.position);
    if (&other != this && dist < neighborDistance) {
      sum += other.position;
      count++;
    }
  }
  if (count > 0) {
    sum /= count;
    return seek(sum);
  }
  return glm::vec3(0, 0, 0);
}

glm::vec3 Boid::fleeCollision(std::vector<std::vector<float>> &heightMap) {
  vector<glm::vec3> collisionRays = getRays();
  int collisionCount = 0;
  glm::vec3 collisionPoint = glm::vec3(0, 0, 0);
  for (auto ray : collisionRays) {
    glm::vec3 endOfRay = position + ray;
    if (checkUnderHeightMap(endOfRay, heightMap)) {
      collisionPoint += endOfRay;
      collisionCount++;
    }
  }
  glm::vec3 fleeCollision = glm::vec3(0, 0, 0);
  if (collisionCount > 0) {
    collisionPoint /= collisionCount;
    if (toggleShowMeshCollision) {
      ofSetColor(ofColor::red);
      ofDrawSphere(collisionPoint, 0.4);
    }
    fleeCollision = flee(collisionPoint);
  }

  return fleeCollision;
}
void Boid::showSeek() {
  ofSetColor(ofColor::green);
  ofDrawSphere(seekPosition, 1);
}
void Boid::applyBehaviors(const vector<Boid> &boids,
                          const vector<Boid> &predators,
                          const vector<Boid> &prey,
                          std::vector<std::vector<float>> &heightMap) {

  glm::vec3 separation = separate(boids);
  glm::vec3 alignment = align(boids);
  glm::vec3 cohesion = cohere(boids);
  glm::vec3 collision = fleeCollision(heightMap);

  glm::vec3 fleePredatorForce = glm::vec3(0, 0, 0);
  glm::vec3 seekPreyForce = glm::vec3(0, 0, 0);
  float healthPercentage = (float)health / maxHealth;

  if (type == "predator") {
    health--;
    if (healthPercentage < 0.8) {
      glm::vec3 preyLocation = glm::vec3(0, 0, 0);
      float minDistance = std::numeric_limits<float>::max();
      for (auto &p : prey) {
        if (glm::distance(position, p.position) < visionRadius) {
          preyLocation = p.position;
          minDistance =
              std::min(minDistance, glm::distance(position, p.position));
        }
      }
      if (minDistance < std::numeric_limits<float>::max()) {
        seekPreyForce = seek(preyLocation);
        seekPosition = preyLocation;
      }
    }

  } else if (type == "prey") { // only happens if it has predators and food
    // avoid predators
    health--;
    glm::vec3 predatorLocation = glm::vec3(0, 0, 0);
    int numPredators = 0;
    for (auto &predator : predators) {
      if (glm::distance(position, predator.position) < visionRadius) {
        predatorLocation += predator.position;
        numPredators++;
      }
    }

    if (numPredators > 0) {
      predatorLocation /= numPredators;
      fleePredatorForce = flee(predatorLocation);
    }

    // Seek Prey (Food)
    if (healthPercentage < 0.8) {
      separationRadius = interactionRadius;
      glm::vec3 preyLocation = glm::vec3(0, 0, 0);
      float minDistance = std::numeric_limits<float>::max();
      for (auto &p : prey) {
        if (glm::distance(position, p.position) < visionRadius) {
          preyLocation = p.position;
          minDistance =
              std::min(minDistance, glm::distance(position, p.position));
        }
      }
      if (minDistance < std::numeric_limits<float>::max()) {
        seekPreyForce = seek(preyLocation);
        seekPosition = preyLocation;
      }
    }
  }

  // Wandering force

  if (checkUnderHeightMap(position, heightMap)) {
    // fishColor = ofColor::red;
    health = 0;
  }

  separation *= 1.3;
  alignment *= 1;
  cohesion *= 1;
  collision *= 4;
  fleePredatorForce *= 2.5;
  seekPreyForce *= 2.5;

  applyForce(separation);
  applyForce(alignment);
  applyForce(cohesion);
  applyForce(collision);
  applyForce(fleePredatorForce);
  applyForce(seekPreyForce);
}

void Boid::checkEdges() {
  int BOX_LENGTH = 375;
  if (position.x > BOX_LENGTH) {
    position.x = -BOX_LENGTH;
  } else if (position.x < -BOX_LENGTH) {
    position.x = BOX_LENGTH;
  }
  if (position.y > 0) {
    position.y = -100;
  } else if (position.y < -100) {
    position.y = 0;
  }
  if (position.z > BOX_LENGTH) {
    position.z = -BOX_LENGTH;
  } else if (position.z < -BOX_LENGTH) {
    position.z = BOX_LENGTH;
  }
}
// cout << "x: " << position.x << " y: " << position.y << " z: " << position.z
// << endl;
void Boid::updateParams(const BoidParams &params, const Features &features) {
  if (type == "prey") {
    maxSpeed = params.preyMaxSpeed;
    maxForce = params.preyMaxForce;
    visionRadius = params.preyVisionRadius;
  } else if (type == "predator") {
    maxSpeed = params.predatorMaxSpeed;
    maxForce = params.predatorMaxForce;
    visionRadius = params.predatorVisionRadius;
  }
  interactionRadius = params.interactionRadius;
  separationRadius = params.separationRadius;
  alignmentRadius = params.alignmentRadius;
  cohesionRadius = params.cohesionRadius;

  if (features.enableCollisionRays) {
    toggleShowRays = true;
  } else {
    toggleShowRays = false;
  }
  if (features.enableSeekFoodPoint) {
    toggleShowSeek = true;
  } else {
    toggleShowSeek = false;
  }
  if (features.showMeshCollision) {
    toggleShowMeshCollision = true;
  } else {
    toggleShowMeshCollision = false;
  }
  if (features.showHealth) {
    toggleHealth = true;
  } else {
    toggleHealth = false;
  }
}

bool Boid::checkUnderHeightMap(glm::vec3 pos,
                               std::vector<std::vector<float>> &heightMap) {
  int boidMin = -375;
  int boidMax = 375;
  int heightRangeMin = 0;
  int heightRangeMax = 99;

  int x = ofMap(pos.x, boidMin, boidMax, heightRangeMin, heightRangeMax); // x
  int z = ofMap(pos.z, boidMin, boidMax, heightRangeMin, heightRangeMax); // x
  x = ofClamp(x, 0, heightMap[0].size() - 1);
  z = ofClamp(z, 0, heightMap.size() - 1);

  if (pos.y <= heightMap[z][x] || pos.y >= 5) {
    return true;
  }
  return false;
}

void Boid::checkInteraction(vector<Boid> &predators) {
  for (auto predator : predators) {
    if (glm::distance(position, predator.position) < interactionRadius) {
      health = 0;
    }
    predator.health += 2000;
  }
}