#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    setupParams();
    gui.setup(params);
    gui.setPosition(80,80);

    //ofSetVerticalSync(true);
    ofBackground(66);
    ofEnableAlphaBlending();
    ofSetFrameRate(25);

    inputs.resize(NUM_VARS);

    colors.clear();
    colors.resize(NUM_PLOTS);

    colors[0]  = ofColor::yellow;
    colors[1]  = ofColor::yellow;
    colors[2]  = ofColor::green;
    colors[3]  = ofColor::green;
    colors[4]  = ofColor::blue;
    colors[5]  = ofColor::blue;
    colors[6]  = ofColor::red;
    colors[7]  = ofColor::red;
    colors[8]  = ofColor::orange;
    colors[9]  = ofColor::orange;


    //    for(int i = 0 ; i < NUM_PLOTS; i = i + 2)
    //    {
    //        float percent = i / (float)(NUM_PLOTS - 1) ;
    //        ofColor c;
    //        c.setHue(percent * 255);
    //        c.setSaturation(1*255);
    //        c.setBrightness(1*255);
    //
    //        //        colors[i].setHue((i/(float)NUM_PLOTS)*255);
    //        //        colors[i].setSaturation(1*255);
    //        //        colors[i].setBrightness(1*255);
    //        //        colors[i+1]=colors[i];
    //
    //        colors.push_back(c);
    //    }


    //    float minS = 0.01;
    //    float maxS = 0.8;

    for(int i = 0 ; i < NUM_PLOTS; i++){
        string _name;
        if (i%2==0) _name = "Input"+ ofToString(i/2);
        else _name = "Output"+ ofToString(i/2);

        plot[i] = new ofxHistoryPlot(NULL,_name, 200, false);
//        plot[i]->setRange(-0.2, 1.2);
        plot[i]->setRange(0, 1);

        plot[i]->setColor(colors[i]);
        //plot[i]->setGridUnit(10);
        plot[i]->setShowNumericalInfo(true);

        //float s = minS + (maxS - minS) * percent * percent;
        //cout << "setting up filter with speed :" << s << endl;
        //filter[i].setup(s);
    }


    outputs.resize(NUM_VARS);
    for(int i = 0 ; i < NUM_VARS; i++)
    {
        outputs[i].initAccum(100);
        //specAmps[i].initSlide(60, 20);
//        specAmps[i].setOutputRange(ofVec2f(0.2, 0.8));
        //specAmps[i].setNormalized(true, ofVec2f(0.2, 0.8));
        outputs[i].directionChangeCalculated = true;
        outputs[i].setBonk(0.1, 0.1);
    }

    //            amp.initSlide(60, 20); // slide up, slide down
    ////    initializing accumulator smoothing:
    //
    //    amp.initAccum(100); // size of history
    ////    initializing clamping and/or normalization
    //
    ////    amp.setOutputRange(0.2, 0.8); // clamp min and max (default 0 and 1)
    ////    amp.setNormalized(true, 0.2, 0.8); // normalization, min, max, clamped (optional, default true)
    ////    set a threshold:
    //
    //    amp.setThresh(0.5);
    ////    get a threshold crossing:
    //
    //    amp.getTrigger(0); // index (optional)
    //}

    cbg = ofColor(32);

    doReset();
}

//--------------------------------------------------------------
void ofApp::update(){

    for(int i = 0 ; i < NUM_VARS; i++){
        switch (i) {
            case 0: inputs[i] = (bTrigManual?1:0); break;
            case 1: inputs[i] = ofxSurfingHelpers::Tick((bModeNoise?0.2:1)); break;
            case 2: inputs[i] = ofxSurfingHelpers::Noise(ofPoint((!bModeNoise?1:0.001), (!bModeNoise?1.3:2.3)));break;
            case 3: inputs[i] = ofClamp( ofxSurfingHelpers::NextGaussian(0.5, (bModeNoise?1:0.1)), 0,1);break;
            case 4: inputs[i] = ofxSurfingHelpers::NextReal(0, (bModeNoise?1:0.1));break;
        }

        outputs[i].update(inputs[i]); // raw value, index (optional)
    }

    for(int i = 0 ; i < NUM_VARS; i++)
    {
        float _input = ofClamp(inputs[i], minInput, maxInput);
        plot[2*i]->update(_input);//source

        if (enableSmooth) plot[2*i+1]->update(outputs[i].getValue());//filtered
        else plot[2*i+1]->update(_input);//source

        //TODO:
        if ( i == index){
            input = _input;
        }
    }


    //TODO:
    //output
    if (enableSmooth) {
    if(bNormalized) output = outputs[index].getValueN();
    else output = outputs[index].getValue();
    }

    if (!enableSmooth)
    {
        output = input;
    }

    //    update with incoming values:
    //    amp.update(rawAmplitude, 0); // raw value, index (optional)
    //    specAmps.update(spectralAmplitudes); // or update with a const vector ref
    //    amp.incrUpdate(rawAmplitudeDiff); //increment current value and update

    for(int i = 0 ; i < NUM_VARS; i++){
        //if (i!=0)continue;

        if(outputs[i].getTrigger()) {
            if (i == index) ofLogNotice() << "Trigger: "<<i;
        }

        if(outputs[i].getBonk()) {
            if (i == index) ofLogNotice() << "Bonk: "<<i;
        }

        if (outputs[i].getDirectionTimeDiff() > 0.5 &&
            outputs[i].directionHasChanged()) {
            if (i == index) ofLogNotice() <<
                "Direction: "<<i<<" " << outputs[i].getDirectionTimeDiff() <<
                " " << outputs[i].getDirectionValDiff();
        }
    }

    // if the direction has changed and
    // if the time of change is greater than 0.5 sec
    // print the time between changes and amount of change
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofClear(cbg);

    ofPushStyle();

    int h = ofGetHeight()/NUM_PLOTS;

    int y = 0;
    for(int i = 0 ; i < NUM_PLOTS; i++){
        if (solo && (i/2 != index)) { y += h; continue;}

        plot[i]->draw(0, y , ofGetWidth(), h);

        //draw threshold
        if (i/2 == index){//TODO:
            ofColor c;
            if (outputs[index].getBonk()) c.set(ofColor::white);
            else if (outputs[index].getTrigger()) c.set(ofColor(colors[i], 180));
            else c = ofColor(ofColor(colors[i], 100*ofxSurfingHelpers::Bounce(0.5)));
            ofSetColor(c);
            float yth = y + (1-threshold)*h;
            ofLine(0, yth, ofGetWidth(), yth);
        }

        y += h;
    }

//    //extra
//    y = 0;
//    for(int i = 0 ; i < NUM_PLOTS; i++){
//        if (solo && i/2 != index) {
////            ofSetColor(255, 200);
////             ofLine(0, y, ofGetWidth(), y);
//            y += h;
//            continue;}
//
//        //baseline
////        if ((i/2 == index+1) || (i/2 == index+2)) ofSetColor(255, 200);
//        if ((i/2 == index) ) ofSetColor(255, 200);
//        else ofSetColor(255, 64);
//
//        ofLine(0, y, ofGetWidth(), y);
//
////        ofLine(0, y+h, ofGetWidth(), y+h);
//
//        //name
//        //ofSetColor(128,0,0);
//        //ofDrawBitmapString( "filterVal: " + ofToString(filter[i].getSpeed()), 5, y + 11 );
//
//        y += h;
//    }

    ofPopStyle();


    //    get a smoothed value (update updating):
    //    amp.getValue(0); // index (optional)
    //    amp.getValueN(0); // get normalized value (index optional)
    //    detect "bonks" (onsets):
    //    amp.setBonk(0.1, 0.1);  // min growth for onset, min decay
    //    set growth/decay:
    //    amp.setDecayGrow(true, 0.99); // a framerate-dependent steady decay/growth
    //    get direction change time (sec) and depth:
    // if the direction has changed and
    // if the time of change is greater than 0.5 sec
    // print the time between changes and amount of change
    //    if (inputStreams[0].getDirectionTimeDiff() > 0.5 &&
    //        inputStreams[0].directionHasChanged()) {
    //        ofLogNotice() <<
    //        "inputStream1 " << inputStreams[0].getDirectionTimeDiff() <<
    //        " " << inputStreams[0].getDirectionValDiff();
    //    }

    //-

    gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key == ' ') bTrigManual=!bTrigManual;
    if(key == OF_KEY_RETURN) bModeNoise=!bModeNoise;
    if(key == OF_KEY_UP){
        index--;
        index = ofClamp(index, index.getMin(), index.getMax());
    }
    if(key == OF_KEY_DOWN){
        index++;
        index = ofClamp(index, index.getMin(), index.getMax());
    }
    if(key == 's') solo =!solo;
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

//--------------------------------------------------------------
void ofApp::setupParams() {

    string name = "filter";
//    bool _bNormalized=true;
    float _inputMinRange;
    float _inputMaxRange;
    float _outMinRange;
    float _outMaxRange;


//    if (_bNormalized)
    {
        _inputMinRange = 0;
        _inputMaxRange = 1;
        _outMaxRange = 1;
        _outMinRange = 0;
    }
//    else
//    {
//        _inputMinRange = 0;
//        _inputMaxRange = 1;
//        _outMinRange = -100;
//        _outMaxRange = 100;
//    }

    params.setName(name);
    params.add(enable.set("enable", true));
    params.add(index.set("index", 0, 0, 4));
    params.add(solo.set("solo", false));
    params.add(minInput.set("minInput", 0, _inputMinRange, _inputMaxRange));
    params.add(maxInput.set("maxInput", 1, _inputMinRange, _inputMaxRange));
    params.add(bNormalized.set("bNormalized", false));
    params.add(minOutput.set("minOutput", 0, _outMinRange, _outMaxRange));
    params.add(maxOutput.set("maxOutput", 1, _outMinRange, _outMaxRange));
    params.add(enableSmooth.set("smooth", true));
    params.add(smoothVal.set("smooth power", 0.25, 0.0, 1));
    params.add(slideMin.set("slideIn", 0.2, 0.0, 1));
    params.add(slideMax.set("slideOut", 0.2, 0.0, 1));
    //params.add(bClamp.set("CLAMP", true));
    params.add(onsetGrow.set("onsetGrow", 0.1, 0.0, 1));
    params.add(onsetDecay.set("onsetDecay", 0.1, 0.0, 1));
    params.add(bReset.set("bReset", false));
    params.add(threshold.set("threshold", 0.5, 0.0, 1));
    params.add(input.set("input", 0, _inputMinRange, _inputMaxRange));
    params.add(output.set("output", 0, _outMinRange, _outMaxRange));


    //exclude
    input.setSerializable(false);
    output.setSerializable(false);
    solo.setSerializable(false);
    bReset.setSerializable(false);

    ofAddListener(params.parameterChangedE(), this, &ofApp::Changed_params); // setup()
}

//--------------------------------------------------------------
void ofApp::exit() {
    ofRemoveListener(params.parameterChangedE(), this, &ofApp::Changed_params); // exit()
}

//--------------------------------------------------------------
void ofApp::doReset() {
    ofLogNotice(__FUNCTION__) << "output: " << ofToString(output);
    enable = true;
    enableSmooth = true;
    minInput = 0;
    maxInput = 1;
    minOutput = 0;
    maxOutput = 1;
    slideMin = 0.2;
    slideMax = 0.2;
    onsetGrow = 0.1;
    onsetDecay = 0.1;
    output = 0;
    bNormalized = false;
    smoothVal = 0.25;
    bClamp = true;
}

// callback for a parameter group
//--------------------------------------------------------------
void ofApp::Changed_params(ofAbstractParameter &e)
{
    //    if (!DISABLE_Callbacks)
    {
        string name = e.getName();
        if (name != input.getName() || name != output.getName())
        {
            ofLogNotice() << "Changed parameter named: " << name << " : with value " << e;
        }

        if (name == bReset.getName())
        {
            if (bReset)
            {
                bReset = false;
                doReset();
            }
        }

        if (name == smoothVal.getName())
        {
            int MAX_HISTORY= 30;
            float v = ofMap(smoothVal, 0, 1, 1, MAX_HISTORY);

            for(int i = 0 ; i < NUM_VARS; i++){
                outputs[i].initAccum(v);
            }
        }

        if (name == threshold.getName())
        {
            for(int i = 0 ; i < NUM_VARS; i++){
                outputs[i].setThresh(threshold);
            }
        }

        if (name == minOutput.getName() || name == maxOutput.getName())
        {
            for(int i = 0 ; i < NUM_VARS; i++){
                outputs[i].setOutputRange(ofVec2f(minOutput, maxOutput));
            }
        }

        if (name == bNormalized.getName())
        {
            for(int i = 0 ; i < NUM_VARS; i++){
                //                outputs[i].setOutputRange(ofVec2f(minOutput, maxOutput));

                if (bNormalized) outputs[i].setNormalized(bNormalized, ofVec2f(0, 1));
                else outputs[i].setNormalized(bNormalized, ofVec2f(minOutput, maxOutput));
            }
        }

        if (name == slideMin.getName() || name == slideMax.getName())
        {
            for(int i = 0 ; i < NUM_VARS; i++){
                const int MIN_SLIDE = 1;
                const int MAX_SLIDE = 50;
                float _slmin = ofMap(slideMin,0,1,MIN_SLIDE, MAX_SLIDE, true);
                float _slmax = ofMap(slideMax,0,1,MIN_SLIDE, MAX_SLIDE, true);

                outputs[i].initSlide(_slmin, _slmax);
            }
        }

        //detect "bonks" (onsets):
        //amp.setBonk(0.1, 0.1);  // min growth for onset, min decay
        //set growth/decay:
        //amp.setDecayGrow(true, 0.99); // a framerate-dependent steady decay/growth
        if (name == onsetGrow.getName()||name == onsetDecay.getName())
        {
            for(int i = 0 ; i < NUM_VARS; i++){
                outputs[i].setBonk(onsetGrow, onsetDecay);
                //specAmps[i].setDecayGrow(true, 0.99);
            }
        }
    }
}

