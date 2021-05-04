#pragma once

#include "ofMain.h"
#include "ofxDataStream.h"
#include "ofxSurfing_Timers.h"
#include "ofxHistoryPlot.h"
#include "ofxGui.h"

#define NUM_VARS 5
#define NUM_PLOTS 10

class ofApp : public ofBaseApp{

public:

    ofxHistoryPlot * plot[NUM_PLOTS];

    void setup();
    void update();
    void draw();
    void exit();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    vector<ofxDataStream> specAmps;
    vector<float> filter;

    bool bTrigManual =false;
    bool bModeNoise =false;

    vector<ofColor> colors;

    ofxPanel gui;
    void Changed_params(ofAbstractParameter &e);
    ofParameterGroup params;
    ofParameter<bool> enable;
    ofParameter<bool> solo;
    ofParameter<bool> bClamp;
    ofParameter<float> input;
    ofParameter<float> minInput;
    ofParameter<float> maxInput;
    ofParameter<float> minOutput;
    ofParameter<float> maxOutput;
    ofParameter<bool> enableSmooth;
    ofParameter<float> smoothVal;
    ofParameter<float> output;
    ofParameter<bool> bReset;
    ofParameter<float> threshold;
    ofParameter<float> slideMin;
    ofParameter<float> slideMax;

    void doReset();
    void setupParams();

    ofColor cbg;

};
