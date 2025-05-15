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
#include "ofLight.h"
#include "ofMath.h"
#include "ofTexture.h"
#include "ofUtils.h"
#include "ofVboMesh.h"
#include <cstdlib>

//--------------------------------------------------------------
void ofApp::setup() {
  if (ofIsGLProgrammableRenderer()) {
    cout << "gls3" << endl;
    mainShader.load("shadersGL3/mainShader");
    debugShader.load("shadersGL3/debugShader");
  } else {
    cout << "gls2" << endl;
  }
  // setting up compute shader
  compute.setupShaderFromFile(GL_COMPUTE_SHADER, "particleCompute.glsl");
  compute.linkProgram();
  particles.resize(1024);

  for (auto &p : particles) {
    p.pos.x = pECenterx;
    p.pos.y = pECentery;
    p.pos.z = pECenterz;
    p.pos.w = ofRandom(3);
    p.vel = {ofRandom(-5, 5), 10, ofRandom(-5, 5), 0};
  }
  // setting up the buffers and the vbo. The job of the vbo is to draw on
  // screen.
  particlesBuffer.allocate(particles, GL_DYNAMIC_DRAW);
  particlesBuffer2.allocate(particles, GL_DYNAMIC_DRAW);

  vbo.setVertexBuffer(particlesBuffer, 4, sizeof(Particle));
  vbo.setColorBuffer(particlesBuffer, sizeof(Particle), sizeof(glm::vec4) * 2);
  // vbo.disableColors();
  particlesBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 0);
  particlesBuffer2.bindBase(GL_SHADER_STORAGE_BUFFER, 1);

  cam.setDistance(2);
  cam.setNearClip(0.1);
  cam.setFarClip(500);
  light.setup();
  light.enable();
  // light.setDirectional();
  light.setDiffuseColor(ofColor::lightBlue);
  light.setSpecularColor(ofColor::lightYellow); // like the sun
  ofFile file;
  ofFile f2;
  file.open("simple_terrain.ply", ofFile::ReadOnly);
  f2.open("water_plane.ply", ofFile::ReadOnly);
  terrainMesh.load(file);
  waterPlane.load(f2);

  ofFile s_box;
  s_box.open("skybox.png", ofFile::ReadOnly);
  skybox.load(s_box, 2300, true);

  //                                           0);
  gui.setup();
  gui.add(lightPosX.setup("Light X", -0.2, -50.0, 50.0));
  gui.add(lightPosY.setup("Light Y", 1.3, -50.0, 50.0));
  gui.add(lightPosZ.setup("Light Z", -2.2, -50.0, 50.0));
  gui.add(pECenterx.setup("Emitter Center X", 14, -200, 200));
  gui.add(pECentery.setup("Emitter Center Y", 56, -200, 200));
  gui.add(pECenterz.setup("Emitter Center Z", 37, -200, 200));
  gui.add(pECenterRadius.setup("Emitter Center Radius", 0, 0, 30));
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


  model = glm::mat4(1.0) * glm::scale(glm::vec3(50, 50, 50));
  mainShader.setUniformMatrix4f("model", model);

  terrainMesh.draw();
  waterPlane.draw();

  cam.end();
  mainShader.end();

  cam.begin();
  skybox.draw();
  cam.end();
}



//--------------------------------------------------------------
void ofApp::update() {
  ofEnableDepthTest();

  compute.begin();
  // cout << pECenterx << endl;
  compute.setUniform1f("emitterX", pECenterx);
  compute.setUniform1f("emitterY", pECentery);
  compute.setUniform1f("emitterZ", pECenterz);
  compute.setUniform1f("emitterR", pECenterRadius);

  compute.dispatchCompute((particles.size() + 1024 - 1) / 1024, 1, 1);
  compute.end();
  particlesBuffer.copyTo(particlesBuffer2);
}

void drawNormals(ofVboMesh &mesh) {
  auto normals = mesh.getNormals();
  auto vertices = mesh.getVertices();
  float size = 0.05;
  for (int i = 0; i < vertices.size(); i++) {
    auto start = vertices[i];
    auto end = vertices[i] + normals[i] * size;
    ofDrawLine(start, end);
  }
}

//--------------------------------------------------------------
void ofApp::draw() {

  renderScene();

  cam.begin();
  ofSetColor(ofColor::yellow);
  ofDrawSphere(light.getPosition(),
               1); // Draw the light's position as a small sphere

  // Draw a line from the light to the target
  ofSetColor(ofColor::red);
  ofDrawLine(light.getPosition(),
             glm::vec3(0.0f, 0.0f, 0.0f)); // Line to the target
  ofSetColor(ofColor::red);
  glPointSize(5);
  vbo.draw(GL_POINTS, 0, particles.size());
  cam.end();

  // GUI STUFF
  ofDisableDepthTest();
  gui.draw();
  ofEnableDepthTest();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
  if (key == 'c') {
    std::cout << cam.getPosition() << std::endl;
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