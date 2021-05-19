#pragma once
#include "ofMain.h"

#include "DataStreamGroup.h"
#include "ofxWindowApp.h"
#include "ofxMidiParams.h"
#include "ofxSurfingDebugVariables.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void setupDebugger();
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

		DataStreamGroup dataStreamGroup;

		ofParameterGroup params;
		ofParameter<float> lineWidth;
		ofParameter<float> separation;
		ofParameter<int> shapeType;
		ofParameter<int> amount;
		ofParameter<int> size;
		ofxMidiParams mMidiParams;

		ofxWindowApp windowApp;

		ofParameterGroup paramsApp{ "App" };
		ofxPanel gui;
};
