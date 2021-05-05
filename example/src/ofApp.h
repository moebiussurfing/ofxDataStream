#pragma once

#include "ofMain.h"
#include "ofxDataStream.h"
#include "ofxSurfing_Timers.h"
#include "ofxHistoryPlot.h"
//#include "ofxGui.h"
#include "ofxImGui.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ofxSurfing_ImGui.h"
#include "ofxInteractiveRect.h"

#define NUM_VARS 5
#define NUM_PLOTS 10

class ofApp : public ofBaseApp{

public:
	ofxInteractiveRect rectangle_Plots = { "Rect_Plots" };

    ofxHistoryPlot * plot[NUM_PLOTS];

    vector<ofColor> colors;

    void setup();
    
	void setupPlots();
	void drawPlots(ofRectangle r);

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

    vector<ofxDataStream> outputs;
    vector<float> inputs;

    //ofxPanel gui;

    void Changed_params(ofAbstractParameter &e);

    ofParameterGroup params;
    ofParameter<bool> enable;
    ofParameter<bool> solo;
    ofParameter<int> index;
    ofParameter<int> type;
    ofParameter<string> type_Str;
    ofParameter<bool> bClamp;
    ofParameter<float> minInput;
    ofParameter<float> maxInput;
    ofParameter<bool> bNormalized;
    ofParameter<float> minOutput;
    ofParameter<float> maxOutput;
    ofParameter<bool> enableSmooth;
    ofParameter<float> smoothVal;
    ofParameter<float> threshold;
    ofParameter<float> onsetGrow;
    ofParameter<float> onsetDecay;
    ofParameter<float> slideMin;
    ofParameter<float> slideMax;
    ofParameter<float> input;
    ofParameter<float> output;
    ofParameter<bool> bReset;

    void doReset();
    void setupParams();

    bool bTrigManual =false;//flip first
    bool bModeNoise =false;//fast generators

    ofColor colorBg;

	void setup_ImGui();
	void draw_ImGui();
	ofxImGui::Gui gui;
	ofxImGui::Settings mainSettings = ofxImGui::Settings();
	ImFont* customFont = nullptr;
	ofParameter<bool> bGui{ "Show Gui", true };
	ofParameter<bool> auto_resize{ "Auto Resize", true};
	ofParameter<bool> bLockMouseByImGui{ "Mouse Locked", false };
	ofParameter<bool> auto_lockToBorder{ "Lock GUI", false };


	//enum Smoothing_t {
	//	SMOOTHING_NONE,
	//	SMOOTHING_ACCUM,
	//	SMOOTHING_SLIDE
	//} smoothingType;

	//enum Mean_t {
	//	MEAN_ARITH,
	//	MEAN_GEOM,
	//	MEAN_HARM,
	//} meanType;

	std::vector<std::string> typeLabels;
};
