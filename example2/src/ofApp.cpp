#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	//windowApp.setup();

	params.setName("ParamsGroup");//group 
	params.add(lineWidth.set("lineWidth", 1, 0.1, 10));
	params.add(shapeType.set("shapeType", 1, 1, 5));
	params.add(size.set("size", 100, 5, 200));
	params.add(amount.set("amount", 10, 1, 24));
	params.add(separation.set("separation", 10, 1, 100));

	dataStreamGroup.setup();
	dataStreamGroup.addGroup(params);

	mMidiParams.connect(1, true);
	mMidiParams.add(params);
	mMidiParams.setPosition(ofGetWidth() - 320, 20);
	mMidiParams.load("example-midi-params.xml");
}

//--------------------------------------------------------------
void ofApp::update(){
	dataStreamGroup.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
	dataStreamGroup.draw();
	mMidiParams.draw();
}

//--------------------------------------------------------------
void ofApp::exit(){
	dataStreamGroup.exit();
	//mMidiParams.exit();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	dataStreamGroup.keyPressed(key);
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
