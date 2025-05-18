#include "Boid.hpp"
#include "quaternion.hpp"

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
      glm::vec3(ofRandom(-0.1, 0.1), ofRandom(-0.1, 0.1), ofRandom(-0.1, 0.1));
  acceleration =
      glm::vec3(ofRandom(-0.1, 0.1), ofRandom(-0.1, 0.1), ofRandom(-0.1, 0.1));
  position = glm::vec3(ofRandom(-50, 50), ofRandom(-100, 0), ofRandom(-50, 50));

  if (ofRandom(1) < 0.5) {
    // Generate a random shade of orange
    fishColor.setHsb(ofRandom(20, 40), ofRandom(150, 255), ofRandom(150, 255));
  } else {
    // Generate a random shade of blue
    fishColor.setHsb(ofRandom(190, 210), ofRandom(150, 255),
                     ofRandom(150, 255));
  }
}

void Boid::draw(ofx::assimp::Model &model) {
  model.enableColors();
  
  ofSetColor(fishColor);
  // cout << fishColor << endl;
  if (glm::length(velocity) > 0) {
    glm::vec3 dir = glm::normalize(velocity);
    glm::vec3 coneDir = glm::vec3(0, 0, -1);

    glm::mat4 rotationMatrix = rotateToVector(coneDir, dir);
    glm::mat4 coneTransform =
        glm::translate(glm::mat4(1.0f), position) * rotationMatrix;
    coneTransform = glm::scale(coneTransform, glm::vec3(0.25, 0.25, 0.25));
    ofPushMatrix();
    ofMultMatrix(coneTransform);
    model.draw();
    // ofDrawCone(0, 0, 0, 0.3, 1.0);
    ofPopMatrix();

    // drawing rays for each boid
    showRays();
    
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

void Boid::flee(glm::vec3 target) {
  glm::vec3 desired = -(target - position);
  glm::vec3 steer = glm::normalize(desired) * 0.05;
  if (glm::length(steer) > maxForce) {
    steer = glm::normalize(steer) * maxForce;
  }
  applyForce(steer);
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
      // cout << "Current Boid Position: " << position.x << ", " << position.y
      // << ", " << position.z << endl; cout << "Other Boid Position: " <<
      // other.position.x << ", " << other.position.y << ", " <<
      // other.position.z << endl;
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

void Boid::applyBehaviors(const vector<Boid> &boids) {
  glm::vec3 separation = separate(boids);
  glm::vec3 alignment = align(boids);
  glm::vec3 cohesion = cohere(boids);

  separation *= 1.5;
  alignment *= 1;
  cohesion *= 1;

  // separation *= sep;
  // alignment *= ali;
  // cohesion *= coh;

  applyForce(separation);
  applyForce(alignment);
  applyForce(cohesion);
}

void Boid::checkEdges() {
  int BOX_LENGTH = 25;
  if (position.x > BOX_LENGTH) {
    position.x = -BOX_LENGTH;
  } else if (position.x < -BOX_LENGTH) {
    position.x = BOX_LENGTH;
  }
  if (position.y > 0) {
    position.y = -20;
  } else if (position.y < -20) {
    position.y = 0;
  }
  if (position.z > BOX_LENGTH) {
    position.z = -BOX_LENGTH;
  } else if (position.z < -BOX_LENGTH) {
    position.z = BOX_LENGTH;
  }
  // cout << "x: " << position.x << " y: " << position.y << " z: " << position.z
  // << endl;
}