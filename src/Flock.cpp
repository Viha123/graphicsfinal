#include "Flock.hpp"

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

void Flock::update() {}

void Flock::draw(const vector<Boid> &predators, const vector<Boid> &prey,
                 std::vector<std::vector<float>> &heightMap) {
  // Remove dead boids`

  auto it = std::remove_if(boids.begin(), boids.end(),
                           [](Boid &b) { return b.health <= 0; });
//   cout << boids.size() << endl;
  boids.erase(it, boids.end());
//   cout << boids.size()<< endl;

  // Set Predator and Prey vectors

  for (auto &boid : boids) {
    boid.applyBehaviors(boids, predators, prey,
                        heightMap); // TODO move this into update lmfao

    // cout << "finished applied behaviros" << endl;
    boid.checkInteraction(predators);
    // cout << "checked ineractions" << endl;

    boid.update();
    // cout << "finished update " << endl;
    // cout << model << endl;
    boid.draw(model);
  }

//   if (type == "food") {
//     cout << "fin drw" << endl;
//   }
}