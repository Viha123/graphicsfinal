#pragma once

#include "ofBufferObject.h"
#include "ofCubeMap.h"
#include "ofFbo.h"
#include "ofGraphicsBaseTypes.h"
#include "ofMain.h"
#include "ofShader.h"
#include "ofVbo.h"
#include "ofVboMesh.h"
#include "ofxGui.h"
#include "ofxInputField.h"
#include "ofxPanel.h"
#include "ofxSlider.h"
#include <vector>
class ofApp : public ofBaseApp {
public:
  void setup();
  void update();
  void draw();

  void keyPressed(int key);
  void keyReleased(int key);
  void mouseMoved(int x, int y);
  void mouseDragged(int x, int y, int button);
  void mousePressed(int x, int y, int button);
  void mouseReleased(int x, int y, int button);
  void windowResized(int w, int h);
  void dragEvent(ofDragInfo dragInfo);
  void gotMessage(ofMessage msg);
  void renderDepthMap();
  void renderSceneWithShadows();
  void renderSceneFirstPass();
  void renderScene(ofShader& shader);
  ofShader mainShader;
  ofShader shadowShader;
  ofShader debugShader;
  ofShader compute;
  ofEasyCam cam;
  ofLight light;
  ofMesh terrainMesh;
  ofMesh waterPlane;
  ofCubeMap skybox;
  ofTexture depthBufferTexture;

  ofFbo shadow;

  ofxPanel gui;
  ofxFloatSlider lightPosX;
  ofxFloatSlider lightPosY;
  ofxFloatSlider lightPosZ;
  ofxFloatSlider pECenterx;
  ofxFloatSlider pECentery;
  ofxFloatSlider pECenterz;
  ofxFloatSlider pECenterRadius;

  struct Particle { // include lifespan and stuff later
    glm::vec4 pos;
    glm::vec4 vel;
    ofFloatColor col;
    
  };
  std::vector<Particle> particles;
  ofVbo vbo;
  ofBufferObject particlesBuffer,
      particlesBuffer2; // keep track of current pos and vel, and keep track of
                        // prev pos and vel
  
  
};
