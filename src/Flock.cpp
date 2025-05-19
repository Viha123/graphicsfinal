#include "Flock.hpp"
#include "Boid.hpp"

Flock::Flock() {
  if (type == "prey") {
    model.load("fish.obj");
  } else if (type == "predator") {
    model.load("fish.obj");
    model.setScale(2, 2, 2);
  }
  // model.loadModel("fish.obj");
  // model.setScale(0.8, 0.8, 0.8);
  model.disableMaterials();
  model.disableTextures();
  // model.setScaleNormalization(false);
}

void Flock::generateFlock(int numBoids) {
  for (int i = 0; i < numBoids; i++) {
    Boid boid;
    if (type == "predator") {
      boid.type = "predator";
      boid.fishColor = ofColor::red;
      boid.maxSpeed = 0.2;
      boid.maxForce = 0.003;
      boid.visionRadius = 50.0;
      // pass in different flocking params
      // change vision radius
    }
    if (type == "food") {
      boid.type = "food";
      boid.fishColor = ofColor::green;
      boid.maxSpeed = 0;
      boid.maxForce = 0;
      boid.visionRadius = 0;
    }

    boids.push_back(boid);
  }
}

void Flock::add(const Boid &b) { boids.push_back(b); }

void Flock::remove(int i) { boids.erase(boids.begin() + i); }

void Flock::update(const Boid::BoidParams &params, const Boid::Features &features) {
  for (auto &boid : boids) {
    boid.updateParams(params, features);
  }
}

void Flock::draw(vector<Boid> &predators, const vector<Boid> &prey,
                 std::vector<std::vector<float>> &heightMap) {
  // Remove dead boids

  if (boids.size()) {
    boids.erase(
        std::remove_if(boids.begin(), boids.end(),
                       [](const Boid &boid) { return boid.health <= 0; }),
        boids.end());
  }

  // Set Predator and Prey vectors

  for (auto &boid : boids) {
    boid.applyBehaviors(boids, predators, prey,
                        heightMap); // TODO move this into update lmfao
    boid.checkInteraction(predators);
    boid.update();
    boid.draw(model);
  }
}