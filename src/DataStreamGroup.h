#pragma once

/*

TODO:

+ try nested groups
+ add param to calibrate max history smooth
+ add colors types, vectors, templates...

*/


#include "ofMain.h"
#include "ofxDataStream.h"
#include "ofxHistoryPlot.h"
#include "ofxImGui.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ofxSurfingHelpers.h"
#include "ofxSurfing_Timers.h"
#include "ofxSurfing_ImGui.h"
#include "ofxInteractiveRect.h"

#define NUM_GENERATORS 6

#define COLORS_MONCHROME

class DataStreamGroup : public ofBaseApp {

	//private:
	//	enum ParamType {
	//		PTYPE_FLOAT = 0,
	//		PTYPE_INT,
	//		PTYPE_BOOL,
	//		PTYPE_UNKNOWN
	//	};
	//
	//private:
	//	class MidiParamAssoc {
	//	public:
	//		//int midiId = -1;
	//		int paramIndex = 0;
	//		ParamType ptype = PTYPE_UNKNOWN;
	//		//ofRectangle drawRect;
	//		string displayMidiName = "";
	//		//bool bListening = false;
	//		//bool bNeedsTextPrompt = false;
	//		string xmlParentName = "";
	//		string xmlName = "";
	//	}; 
	//vector< shared_ptr<MidiParamAssoc> > mAssocParams;

private:
	ofParameterGroup mParamsGroup;
	void Changed_Controls_Out(ofAbstractParameter &e);

	// api initializers
public:
	void addGroup(ofParameterGroup& aparams);
	void add(ofParameterGroup aparams);
	void add(ofParameter<float>& aparam);
	void add(ofParameter<bool>& aparam);
	void add(ofParameter<int>& aparam);
	void addParam(ofAbstractParameter& aparam);

	// api getters
public:
	float getParamFloatValue(ofAbstractParameter &e);
	int getParamIntValue(ofAbstractParameter &e);
	ofAbstractParameter& getParam(ofAbstractParameter &e);
	ofAbstractParameter& getParam(string name);
	ofParameter<float>& getParamFloat(string name);
	ofParameter<int>& getParamInt(string name);

	void doRandomize();

	//----

public:

	void setup();
	void startup();

	void setupPlots();
	void drawPlots(ofRectangle r);

	void update();

	void updateGenerators();
	void updateSmooths();
	void updatePlots();

	void draw();
	void exit();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
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
	vector<float> generators;

	string path_Settings = "DataStreamGroup.xml";

	ofxInteractiveRect rectangle_Plots = { "Rect_Plots" };

	int NUM_PLOTS;
	int NUM_VARS;

	vector<ofxHistoryPlot *> plots;
	vector<ofColor> colors;

#ifdef COLORS_MONCHROME
	ofColor colorPlots = (ofColor::green);
	//ofColor colorPlots = (ofColor::yellow);
	//ofColor colorPlots = (ofColor::white);
#endif
	ofColor colorSelected;
	ofColor colorBaseLine;

	//ofxPanel gui;

	void Changed_Params(ofAbstractParameter &e);

	ofParameterGroup params;
	ofParameter<bool> enable;
	ofParameter<bool> bFullScreen;
	ofParameter<bool> bShowPlots;
	ofParameter<bool> bUseGenerators;
	ofParameter<bool> solo;
	ofParameter<int> index;
	ofParameter<int> typeSmooth;
	ofParameter<string> typeSmooth_Str;
	ofParameter<int> typeMean;
	ofParameter<string> typeMean_Str;
	ofParameter<bool> bClamp;
	ofParameter<float> minInput;
	ofParameter<float> maxInput;
	ofParameter<bool> bNormalized;
	ofParameter<float> minOutput;
	ofParameter<float> maxOutput;
	ofParameter<bool> enableSmooth;
	ofParameter<float> smoothPower;
	ofParameter<float> threshold;
	ofParameter<float> onsetGrow;
	ofParameter<float> onsetDecay;
	ofParameter<float> slideMin;
	ofParameter<float> slideMax;
	ofParameter<float> input;//index selected
	ofParameter<float> output;
	ofParameter<bool> bReset;
	ofParameter<bool> bPlay;
	float playSpeed;

	int tf;
	float tn;

	void doReset();
	void setupParams();

	bool bTrigManual = false;//flip first
	bool bModeFast = false;//fast generators

	ofColor colorBg;

	void setup_ImGui();
	void draw_ImGui();
	ofxImGui::Gui gui;
	ofxImGui::Settings mainSettings = ofxImGui::Settings();
	ImFont* customFont = nullptr;
	ofParameter<bool> bGui{ "Show ImGui", true };
	ofParameter<bool> auto_resize{ "Auto Resize", true };
	ofParameter<bool> bLockMouseByImGui{ "Mouse Locked", false };
	ofParameter<bool> auto_lockToBorder{ "Lock GUI", false };

public:
	ofParameter<bool> bShowGui{ "SHOW DATA STREAM", true };

	std::vector<std::string> typeSmoothLabels;
	std::vector<std::string> typeMeanLabels;

	bool bDISABLE_CALLBACKS = true;
};
