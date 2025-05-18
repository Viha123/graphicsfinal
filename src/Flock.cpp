#include "Flock.hpp"

Flock::Flock() {
    model.load("fish.obj");
    // model.loadModel("fish.obj");
    // model.setScale(0.8, 0.8, 0.8);
    model.disableMaterials();
    model.disableTextures();
    // model.setScaleNormalization(false);
}

void Flock::generateFlock(int numBoids) {
    for (int i = 0; i < numBoids; i++) {
        Boid boid;
        boids.push_back(boid);
    }
}

void Flock::add(const Boid& b) {
    boids.push_back(b);
}

void Flock::remove(int i) {
    boids.erase(boids.begin() + i);
}

void Flock::update() {
}

void Flock::draw(std::vector<std::vector<float>>& heightMap) {
    for (auto& boid : boids) { 
        boid.applyBehaviors(boids, heightMap); // TODO move this into update lmfao
        boid.update();
        boid.draw(model);
    }
}