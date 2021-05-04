#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    setupParams();
    gui.setup(params);

    //ofSetVerticalSync(true);
    ofBackground(66);
    ofEnableAlphaBlending();
    ofSetFrameRate(25);

    filter.resize(NUM_VARS);


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
        //        float percent = i / (float)(NUM_PLOTS - 1) ;

        plot[i] = new ofxHistoryPlot(NULL,"input", 200, false);
        plot[i]->setRange(-0.2, 1.2);

        //        plot[i]->setColor(ofColor(ofRandom(64,255), ofRandom(64,255), ofRandom(64,255) ));

        plot[i]->setColor(colors[i]);
        //        plot[i]->setColor();

        //        plot[i]->setGridUnit(10);
        plot[i]->setShowNumericalInfo(true);

        //        float s = minS + (maxS - minS) * percent * percent;
        //        cout << "setting up filter with speed :" << s << endl;
        //        filter[i].setup(s);
    }


    specAmps.resize(NUM_VARS);
    for(int i = 0 ; i < NUM_VARS; i++)
    {
        specAmps[i].initAccum(100);
//        specAmps[i].initSlide(60, 20);
        //specAmps[i].setOutputRange(ofVec2f(0.2, 0.8));
        //specAmps[i].setNormalized(true, ofVec2f(0.2, 0.8));
//        specAmps[i].setNormalized(true, ofVec2f(0.2, 0.8));
        specAmps[i].directionChangeCalculated = true;
        specAmps[i].setBonk(0.1, 0.1);
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

    cbg = ofColor(0);

    doReset();
}

//--------------------------------------------------------------
void ofApp::update(){

    for(int i = 0 ; i < NUM_VARS; i++){
        switch (i) {
            case 0: filter[i] = (bTrigManual?1:0); break;
            case 1: filter[i] = ofxSurfingHelpers::Tick((bModeNoise?0.2:1)); break;
            case 2: filter[i] = ofxSurfingHelpers::Noise(ofPoint((!bModeNoise?1:0.001), (!bModeNoise?1.3:2.3)));break;
            case 3: filter[i] = ofClamp( ofxSurfingHelpers::NextGaussian(0.5, (bModeNoise?1:0.1)), 0,1);break;
            case 4: filter[i] = ofxSurfingHelpers::NextReal(0, (bModeNoise?1:0.1));break;
        }

        //specAmps[i] = filter[i];
        specAmps[i].update(filter[i]); // raw value, index (optional)
    }


    for(int i = 0 ; i < NUM_VARS; i++)
    {
        //        float t=ofGetElapsedTimef();
        //        float sinOfTime = ofMap(sin(t*(i+1)),-1,1,0,1 );
        //        auto f = ofGetFrameNum() % 25;
        //        auto t = ofGetElapsedTimeMillis();
        //        filter[i] = ofNoise(i/NUM_PLOTS);
        //        filter[i] = ofRandom(1);
        //        filter[i] = ofNoise(t*i/NUM_PLOTS);

        plot[2*i]->update(filter[i]);//source
        plot[2*i+1]->update(specAmps[i].getValue());//filtered

    }


    //    update with incoming values:

    //    amp.update(rawAmplitude, 0); // raw value, index (optional)
    //    specAmps.update(spectralAmplitudes); // or update with a const vector ref
    //    amp.incrUpdate(rawAmplitudeDiff); //increment current value and update


    for(int i = 0 ; i < NUM_VARS; i++){
        if ( i!=0)continue;

        if(specAmps[i].getTrigger()) {
            cbg = ofColor::red;
            ofLogNotice() << "Trigger: "<<i;
        }

        if(specAmps[i].getBonk()) {
            cbg = ofColor::red;
            ofLogNotice() << "Bonk: "<<i;
        }

        if (specAmps[i].getDirectionTimeDiff() > 0.5 &&
            specAmps[i].directionHasChanged()) {
            ofLogNotice() <<
            "Direction: "<<i<<" " << specAmps[i].getDirectionTimeDiff() <<
            " " << specAmps[i].getDirectionValDiff();
        }
    }


    // if the direction has changed and
    // if the time of change is greater than 0.5 sec
    // print the time between changes and amount of change
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofClear (cbg);

    int h = ofGetHeight()/NUM_PLOTS;
    int y = 0;
    for(int i = 0 ; i < NUM_PLOTS; i++){
        //        ofSetColor(128,0,0);
        plot[i]->draw(0, y , ofGetWidth(), h);
        y += h;
    }
    y = 0;
    //    of rSetColor(64);

    for(int i = 0 ; i < NUM_PLOTS; i++){
        //        ofSetColor(128);
        ofLine(0, y, ofGetWidth(), y);
        //        ofSetColor(128,0,0);
        //        ofDrawBitmapString( "filterVal: " + ofToString(filter[i].getSpeed()), 5, y + 11 );
        y += h;

    }




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

    gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key == ' ') bTrigManual=!bTrigManual;
    if(key == OF_KEY_RETURN) bModeNoise=!bModeNoise;
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
    bool bNormalized=true;
    float inputMinRange;
    float inputMaxRange;
    float outMinRange;
    float outMaxRange;


    if (bNormalized) {
        inputMinRange = 0;
        inputMaxRange = 1;
        outMaxRange = 1;
        outMinRange = 0;
    }
    else
    {
        inputMinRange = 0;
        inputMaxRange = 1;
        outMinRange = -100;
        outMaxRange = 100;
    }

    params.setName(name);
    params.add(enable.set("enable", true));
    params.add(solo.set("solo", false));
    params.add(bReset.set("bReset", false));

    //params.add(input.set("INPUT", 0, inputMinRange, inputMaxRange));
    //params.add(minInput.set("MIN IN", 0, inputMinRange, inputMaxRange));
    //params.add(maxInput.set("MAX IN", 1, inputMinRange, inputMaxRange));

    params.add(minOutput.set("minOutput", 0, outMinRange, outMaxRange));
    params.add(maxOutput.set("maxOutput", 1, outMinRange, outMaxRange));
    params.add(output.set("output", 0, outMinRange, outMaxRange));

    params.add(enableSmooth.set("smooth enable", false));
    params.add(smoothVal.set("smooth power", 0.25, 0.0, 1));
    params.add(threshold.set("threshold", 0.5, 0.0, 1));
    params.add(slideMin.set("slideMin", 0.2, 0.0, 1));
    params.add(slideMax.set("slideMax", 0.8, 0.0, 1));
    //params.add(bClamp.set("CLAMP", true));

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
    minInput = 0;
    maxInput = 1;
    minOutput = 0;
    maxOutput = 1;
    slideMin = 0.2;
    slideMax = 0.8;

    enableSmooth = false;
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
        ofLogNotice() << "Changed parameter named: " << name << " : with value " << e;

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
                specAmps[i].initAccum(v);
            }
        }

        if (name == threshold.getName())
        {
            for(int i = 0 ; i < NUM_VARS; i++){
                specAmps[i].setThresh(threshold);
            }
        }

        if (name == minOutput.getName() || name == maxOutput.getName())
        {
            for(int i = 0 ; i < NUM_VARS; i++){
                specAmps[i].setOutputRange(ofVec2f(minOutput, maxOutput));
                specAmps[i].setNormalized(true, ofVec2f(minOutput, maxOutput));
            }
        }

        if (name == slideMin.getName() || name == slideMax.getName())
        {
            for(int i = 0 ; i < NUM_VARS; i++){
                const int MIN_SLIDE = 1;
                const int MAX_SLIDE = 50;
                float _slmin = ofMap(slideMin,0,1,MIN_SLIDE, MAX_SLIDE, true);
                float _slmax = ofMap(slideMax,0,1,MIN_SLIDE, MAX_SLIDE, true);

                specAmps[i].initSlide(_slmin, _slmax);

//                specAmps[i].setBonk(0.1, 0.1);
                //specAmps[i].setBonk(slideMin, slideMax);
                // min growth for onset, min decay
            }
        }


    }
}

