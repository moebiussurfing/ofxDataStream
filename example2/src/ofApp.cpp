#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	
	params.setName("ParamsGroup"); 
	params.add(lineWidth.set("lineWidth", 0.5, 0, 1));
	params.add(separation.set("separation", 50, 1, 100));
	params.add(shapeType.set("shapeType", 0, -50, 50));
	params.add(size.set("size", 100, 0, 100));
	params.add(amount.set("amount", 10, 0, 25));

	dataStreamGroup.setup();
	dataStreamGroup.addGroup(params);

	mMidiParams.connect(1, true);
	mMidiParams.add(params);
	mMidiParams.load("example-midi-params.xml");

	//ofSetLogLevel(OF_LOG_WARNING);

	setupDebugger();

	//--

	gui.setup();
	gui.add(mMidiParams.bShowGui);
	gui.add(dataStreamGroup.bShowGui);
	gui.add(ofxSurfingDebugVariables::getParamGui());
	//gui.add(ofxSurfingDebugVariables::bShowGui);
}

//--------------------------------------------------------------
void ofApp::update() {
	dataStreamGroup.update();

	//// get smoothed values
	//ofLogNotice(__FUNCTION__) << " " << lineWidth.getName() << " : " << dataStreamGroup.getParamFloatValue(lineWidth);
	//ofLogNotice(__FUNCTION__) << " " << amount.getName() << " : " << dataStreamGroup.getParamIntValue(amount);

	//----

	//string name = lineWidth.getName();
	//auto &p = dataStreamGroup.getParam(name);
	//ofLogNotice(__FUNCTION__) << name << " : ";

	//string name = lineWidth.getName();
	//ofLogNotice(__FUNCTION__) << " " << name << " : " << dataStreamGroup.getParamFloat(name).get();

	//string name = lineWidth.getName();
	//ofLogNotice(__FUNCTION__) << " " << name << " : " << dataStreamGroup.getParamFloatValue(name);


	//ofxSurfingDebugVariables::addText("hello");

}

//--------------------------------------------------------------
void ofApp::draw() {
	dataStreamGroup.draw();
	mMidiParams.draw();

	gui.draw();
}

//--------------------------------------------------------------
void ofApp::exit() {
	dataStreamGroup.exit();

	ofxSurfingDebugVariables::exit();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	dataStreamGroup.keyPressed(key);
	if (key == 'm') { mMidiParams.toggleVisible(); }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}

//--------------------------------------------------------------
void ofApp::setupDebugger() {

	ofxSurfingDebugVariables::setTitle("DataStreamGroup");
	ofxSurfingDebugVariables::addNewLine();

	ofxSurfingDebugVariables::addParamFloat(lineWidth);
	ofxSurfingDebugVariables::addParamFloat(separation);
	ofxSurfingDebugVariables::addParamInt(shapeType);
	ofxSurfingDebugVariables::addParamInt(size);
	ofxSurfingDebugVariables::addParamInt(amount);

	//ofxSurfingDebugVariables::addNewLine();
	//ofxSurfingDebugVariables::addInt("myInt2 (seconds)", &i2);
	//ofxSurfingDebugVariables::addFloat("myFloat4 ", &f4);
	//ofxSurfingDebugVariables::addBool("myBool2", &b2);

	//----

	// cutomization

	// show box. hidden by default
	ofxSurfingDebugVariables::setShowing(true);

	int fontSize = 9;
	string path = "assets/fonts/";
	path += "overpass-mono-bold.otf";
	//path += "mono.ttf";
	ofxSurfingDebugVariables::loadFont(path, fontSize);
}