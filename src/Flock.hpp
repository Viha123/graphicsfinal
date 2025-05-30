#pragma once

#include "Boid.hpp"
#include "ofMain.h"
#include "ofxAssimpModel.h"

class Flock {
public:
  Flock();
  void draw(vector<Boid> &predators, const vector<Boid>& prey, std::vector<std::vector<float>>& heightMap);
  void add(const Boid &); // so that we can insert a pet :sob:
  void remove(int i);     // based on indexing, what if it's just the amount?
  // for now we want infinite lifespan particles
  void reset();  // used to set all forces to zero? or unapplied
  void update(const Boid::BoidParams &params, const Boid::Features& features); // remove those past lifespan and
                                               // udpate particle forces
  
  void applyForces();
  void generateFlock(int numBoids);

  vector<Boid> boids;

  ofx::assimp::Model model;
  std::string type = "prey";
};