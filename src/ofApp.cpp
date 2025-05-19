#include "ofApp.h"
#include "matrix_transform.hpp"
#include "of3dGraphics.h"
#include "ofAppRunner.h"
#include "ofColor.h"
#include "ofEvents.h"
#include "ofFbo.h"
#include "ofFileUtils.h"
#include "ofGraphics.h"
#include "ofGraphicsConstants.h"
// #include "ofLight.h"
#include "ofLight.h"
#include "ofMain.h"
#include "ofMath.h"
#include "ofShader.h"
#include "ofTexture.h"
#include "ofUtils.h"
#include "ofVboMesh.h"
#include <concepts>
#include <cstdlib>

float calculateOctaveHeight(float amplitude, float frequency, int nOctaves,
                            float x, float y) {
  float height = 0;
  for (int i = 0; i < nOctaves; i++) {
    height += ofNoise(x * frequency, y * frequency) * amplitude;
    amplitude *= 0.5;
    frequency *= 2;
  }
  return height;
}
void ofApp::generatePerlinNoiseMesh() {
  // Generate a grid of vertices
  int width = 50;
  int depth = 50;
  customMesh.clear();
  int numX = (width) * 2; // for 0.5 steps
  int numZ = (depth) * 2;
  heightMap.resize(numZ, std::vector<float>(numX, 0.0f));
  // here we make the points inside our mesh
  // add one vertex to the mesh across our width and height
  // we use these x and y values to set the x and y co-ordinates of the mesh,
  // adding a z value of zero to complete the 3d location of each vertex
  int w = 0;
  int d = 0;
  for (float y = 0; y < depth; y += 0.5, ++w) {
    d = 0;
    for (float x = 0; x < width; x += 0.5, ++d) {
      float rawHeight =
          calculateOctaveHeight(amplitude, frequency, octaves, x, y);
      float height =
          (rawHeight - octaves) * 2.0f * amplitude; // Center and scale
      customMesh.addVertex(ofPoint(x - width / 2., height,
                                   y - depth / 2.)); // mesh index = x + y*width
      // this replicates the pixel array within the camera bitmap...
      // Calculate correct texture coordinates
      float u = x / (width - 1);
      float v = y / (depth - 1);
      u = ofClamp(u, 0.0, 1.0);
      v = ofClamp(v, 0.0, 1.0);
      heightMap[w][d] = height * scale; // Store height for (x, z)

      customMesh.addTexCoord(glm::vec2(u, v)); // add texture coordinates
    }
  }
  // // from:
  // //
  // https://github.com/uwe-creative-technology/CT_toolkit_sessions/blob/master/meshExample/src/ofApp.cpp
  // // here we loop through and join the vertices together as indices to make
  // rows
  // // of triangles to make the wireframe grid
  for (int y = 0; y < d - 1; y++) {
    for (int x = 0; x < w - 1; x++) {
      customMesh.addIndex(x + y * w);       // 0
      customMesh.addIndex((x + 1) + y * w); // 1
      customMesh.addIndex(x + (y + 1) * w); // 10

      customMesh.addIndex((x + 1) + y * w);       // 1
      customMesh.addIndex((x + 1) + (y + 1) * w); // 11
      customMesh.addIndex(x + (y + 1) * w);       // 10
    }
  }
  for (auto face : customMesh.getUniqueFaces()) {

    customMesh.addNormal(face.getFaceNormal());
  }
}
//--------------------------------------------------------------
void ofApp::setup() {
  ofDisableArbTex();
  if (ofIsGLProgrammableRenderer()) {
    cout << "gls3" << endl;
    mainShader.load("shadersGL3/mainShader");
    debugShader.load("shadersGL3/debugShader");
  } else {
    cout << "gls2" << endl;
  }
  gui.setup();
  gui.add(lightPosX.setup("Light X", -0.2, -50.0, 50.0));
  gui.add(lightPosY.setup("Light Y", 1.3, -50.0, 50.0));
  gui.add(lightPosZ.setup("Light Z", -2.2, -50.0, 50.0));
  gui.add(pECenterx.setup("Emitter Center X", 14, -400, 600));
  gui.add(pECentery.setup("Emitter Center Y", 56, -200, 600));
  gui.add(pECenterz.setup("Emitter Center Z", 50, -200, 600));
  gui.add(pECenterRadius.setup("Emitter Center Radius", 0, 0, 30));
  gui.add(amplitude.setup("Amplitude", 2.5, 0, 10));
  gui.add(frequency.setup("Frequency", 0.1, 0, 10));
  gui.add(octaves.setup("Octaves", 1, 0, 10));
  // setting up compute shader
  compute.setupShaderFromFile(GL_COMPUTE_SHADER, "particleCompute.glsl");
  compute.linkProgram();
  particles.resize(1024);
  scale = 15;
  for (auto &p : particles) {
    p.pos.x = pECenterx;
    p.pos.y = pECentery;
    p.pos.z = pECenterz;
    p.pos.w = ofRandom(3);
    // p.vel = {ofRandom(-5, 5), 10, ofRandom(-5, 5), 0};
    // p.pos = glm::vec4(pECenterx, pECentery, pECenterz, 1.0f);
    p.vel = {0.1, 10.0, 0.1, 0.0};
    p.col = {1.0, 1.0, 1.0, 1.0};
    // p.vel = {0,0,0,0};
  }
  // setting up the buffers and the vbo. The job of the vbo is to draw on
  // screen.
  particlesBuffer.allocate(particles, GL_DYNAMIC_DRAW);
  particlesBuffer2.allocate(particles, GL_DYNAMIC_DRAW);

  vbo.setVertexBuffer(particlesBuffer, 4, sizeof(Particle));
  vbo.setColorBuffer(particlesBuffer, sizeof(Particle), sizeof(glm::vec4) * 2);

  particlesBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 0);
  particlesBuffer2.bindBase(GL_SHADER_STORAGE_BUFFER, 1);

  cam.setDistance(2);
  cam.setNearClip(0.1);
  cam.setFarClip(800);
  light.setup();

  light.enable();
  light.setSpotlight(60, 20);
  light.setPosition(210, 330.0, 750);
  light.setDiffuseColor(ofFloatColor(1.0, 0.8, 0.8));
  light.setAmbientColor(ofFloatColor(0.4));

  ofFile f2;
  f2.open("water_plane.ply", ofFile::ReadOnly);
  waterPlane.load(f2);

  ofFile s_box;
  s_box.open("skybox.png", ofFile::ReadOnly);
  skybox.load(s_box, 2300, true);
  if (!grassImage.load("grass.jpeg")) {
    cout << "problem with loading grass texture" << endl;
  }
  if (!rockImage.load("rock_or_grass.jpg")) {
    cout << "problem with loading roock texture" << endl;
  }
  if (!model.load("fish.obj")) {
    cout << "problem with loading fish model" << endl;
  }
  //                                           0);

  generatePerlinNoiseMesh();

  // flock thing  // vbo.disableColors();s
  flock.type = "prey";
  flock.generateFlock(10);
  // setup predators

  predators.type = "predator";
  predators.generateFlock(10);
  food.type = "food";
  food.generateFlock(10);
  // for (auto &predator : predators) {
  //   predator.fishColor = ofColor::red;
  //   predator.maxSpeed = 0.2;
  //   predator.maxForce = 0.003;
  // }

  boundingBox.set(750, 200, 750);
}
void ofApp::renderScene() {
  ofSetColor(255);
  ofEnableDepthTest();
  mainShader.begin();
  glClearDepth(1.0f);          // Clear depth to max
  ofClear(255, 255, 255, 255); // optional, we don't really need color here
  light.setPosition(lightPosX, lightPosY, lightPosZ);
  light.lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

  cam.begin();
  glm::mat4 model = glm::mat4(1.0f);
  glm::mat4 view = cam.getModelViewMatrix();
  glm::mat4 projection = cam.getProjectionMatrix();
  glm::mat4 mvp = projection * view * model;
  glm::vec3 lightColor =
      glm::vec3(light.getDiffuseColor()[0], light.getDiffuseColor()[1],
                light.getDiffuseColor()[2]);
  glm::vec3 lightPos = light.getPosition();
  mainShader.setUniform3f("lightColor", lightColor);
  mainShader.setUniform3f("lightPos", lightPos);
  mainShader.setUniform3f("viewPos", cam.getPosition());
  mainShader.setUniform1f("time", ofGetElapsedTimef());
  mainShader.setUniform2f("u_resolution",
                          glm::vec2(ofGetWindowWidth(), ofGetWindowHeight()));
  mainShader.setUniformMatrix4f("lightSpaceMatrix",
                                light.getGlobalTransformMatrix());
  mainShader.setUniformMatrix4f("customMVPMatrix", mvp);

  model = glm::mat4(1.0) * glm::scale(glm::vec3(scale, scale, scale));
  mainShader.setUniformMatrix4f("model", model);

  mainShader.setUniformTexture("grassTexture", grassImage, 0);
  mainShader.setUniformTexture("rockTexture", rockImage, 1);

  customMesh.draw();

  // model = glm::mat4(1.0) * glm::scale(glm::vec3(150, 150, 150));
  // mainShader.setUniformMatrix4f("model", model);
  // // waterPlane.draw();

  cam.end();
  mainShader.end();

  cam.begin();
  ofDisableLighting();
  light.setPosition(lightPosX, lightPosY, lightPosZ);
  ofDrawSphere(light.getPosition(), 0.1);

  skybox.draw();
  // prey
  flock.draw(predators.boids, food.boids, heightMap);
  // predators
  predators.draw(emptyBoids, flock.boids, heightMap);
  // drawing food
  food.draw(flock.boids, emptyBoids, heightMap);

  boundingBox.drawWireframe();
  cam.end();
}

//--------------------------------------------------------------
void ofApp::update() {
  ofEnableDepthTest();
  generatePerlinNoiseMesh();
  compute.begin();
  // cout << pECenterx << endl;
  compute.setUniform1f("emitterX", pECenterx);
  compute.setUniform1f("emitterY", pECentery);
  compute.setUniform1f("emitterZ", pECenterz);
  compute.setUniform1f("emitterR", pECenterRadius);

  compute.dispatchCompute((particles.size() + 1024 - 1) / 1024, 1, 1);
  compute.end();
  particlesBuffer.copyTo(particlesBuffer2);
  particlesBuffer2.copyTo(particlesBuffer);
}

//--------------------------------------------------------------
void ofApp::draw() {
  // ofSetBackgroundColor(ofColor::black);

  renderScene();

  cam.begin();
  // ofDisableLighting();

  glPointSize(10.0f);                       // Set to your desired size
  vbo.draw(GL_POINTS, 0, particles.size()); // drawing particles
  if (ofGetKeyPressed('d')) {
    grassImage.getTexture().draw(0, 0, 200, 200);
    // cout << grassImage.getColor(0) << endl;
  }
  ofSetColor(ofColor::blue);
  ofDrawSphere(pECenterx, pECentery, pECenterz, 1);
  cam.end();

  ofDisableDepthTest();
  gui.draw();
  ofEnableDepthTest();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
  if (key == 'c') {
    std::cout << cam.getPosition() << std::endl;
  }
  if  (key == 'f') {
    cout  << "food " << endl;
    food.generateFlock(10);
  }
  if (key == 'b') {
    std::cout << "boids" << std::endl;
    flock.generateFlock(10);
  }
  if (key == 'p') {
    std::cout << "predators" << std::endl;
    predators.generateFlock(10);
  }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {}