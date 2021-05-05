#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::drawPlots(ofRectangle r) {

	ofPushStyle();

	int hh = r.getHeight();
	int ww = r.getWidth();
	int x = r.getX();
	int y = r.getY();

	//ofPushMatrix();
	//ofTranslate(x,y)

	int h = hh / NUM_PLOTS;
	for (int i = 0; i < NUM_PLOTS; i++) {
		int ii = i / 2;
		//TODO: draw only soloed
		if (solo && (ii != index)) { y += h; continue; }

		plot[i]->draw(x, y, ww, h);

		ofColor _c1 = ofColor(colors[i]);
		ofColor _c2 = ofColor(colors[i]);
		float _a1 = ofxSurfingHelpers::Bounce(1.0);
		float _a2 = ofxSurfingHelpers::Bounce(0.5);

		// draw threshold
		//TODO:
		//if (ii == index) 
		if (i % 2 != 0)
		{
			ofColor c;
			if (outputs[ii].getBonk()) c.set(ofColor(_c2, 125 * _a1));
			else if (outputs[ii].getTrigger()) c.set(ofColor(_c2, 128 * _a2));
			else c.set(ofColor(_c2, 48));
			ofSetColor(c);
			float yth = y + (1 - threshold)*h;
			ofLine(x, yth, x + ww, yth);
		}

		y += h;
	}

	// extra
	x = r.getX();
	y = r.getY();
	for (int i = 0; i < NUM_PLOTS; i++)
	{
		int ii = i / 2;
		//TODO:
		if (solo && (ii != index)) { y += h; continue; }

		//baseline
		ofSetColor(255, 128);
		ofSetLineWidth(1);
		ofLine(x, y + h, x + ww, y + h);

		//name
		ofSetColor(255, 32);
		ofDrawBitmapString(/*"name: " + */ofToString(i), x+5, y + 11);

		y += h;
	}

	//ofPopMatrix();
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::setupPlots() {

	//rectangle_Plots.setAutoSave(true);
	//rectangle_Plots.enableEdit();
	//rectangle_Plots.setRect(10, 10, 400, 400);

	for (int i = 0; i < NUM_PLOTS; i++) {
		string _name;
		if (i % 2 == 0) _name = "Input" + ofToString(i / 2);
		else _name = "Output" + ofToString(i / 2);

		plot[i] = new ofxHistoryPlot(NULL, _name, 200, false);
		plot[i]->setRange(0, 1);
		plot[i]->setColor(colors[i]);
		plot[i]->setShowNumericalInfo(false);
		plot[i]->setShowSmoothedCurve(false);

		//int h = ofGetHeight() / NUM_PLOTS;
		//plot[i]->setGridUnit(h / 4);
		plot[i]->setDrawGrid(false);
		//plot[i]->setDrawBackground(false);
		plot[i]->setDrawTitle(true);
	}
}

//--------------------------------------------------------------
void ofApp::setup() {
	setupParams();

	//gui.setup(params);
	//gui.setPosition(80, 80);

	ofBackground(66);
	ofEnableAlphaBlending();
	ofSetFrameRate(25);
	
	colorBg = ofColor(32);

	inputs.resize(NUM_VARS);

	colors.clear();
	colors.resize(NUM_PLOTS);
	colors[0] = ofColor::yellow;
	colors[1] = ofColor::yellow;
	colors[2] = ofColor::green;
	colors[3] = ofColor::green;
	colors[4] = ofColor::blue;
	colors[5] = ofColor::blue;
	colors[6] = ofColor::red;
	colors[7] = ofColor::red;
	colors[8] = ofColor::orange;
	colors[9] = ofColor::orange;
	
	setupPlots();

	outputs.resize(NUM_VARS);
	for (int i = 0; i < NUM_VARS; i++)
	{
		//default init
		outputs[i].initAccum(100);
		outputs[i].directionChangeCalculated = true;
		outputs[i].setBonk(0.1, 0.1);
	}

	setup_ImGui();

	doReset();
}

//--------------------------------------------------------------
void ofApp::update() {

	for (int i = 0; i < NUM_VARS; i++) {
		switch (i) {
		case 0: inputs[i] = (bTrigManual ? 1 : 0); break;
		case 1: inputs[i] = ofxSurfingHelpers::Tick((bModeNoise ? 0.2 : 1)); break;
		case 2: inputs[i] = ofxSurfingHelpers::Noise(ofPoint((!bModeNoise ? 1 : 0.001), (!bModeNoise ? 1.3 : 2.3))); break;
		case 3: inputs[i] = ofClamp(ofxSurfingHelpers::NextGaussian(0.5, (bModeNoise ? 1 : 0.1)), 0, 1); break;
		case 4: inputs[i] = ofxSurfingHelpers::NextReal(0, (bModeNoise ? 1 : 0.1)); break;
		}

		outputs[i].update(inputs[i]); // raw value, index (optional)
	}

	for (int i = 0; i < NUM_VARS; i++)
	{
		float _input = ofClamp(inputs[i], minInput, maxInput);
		plot[2 * i]->update(_input);//source

		if (enableSmooth) plot[2 * i + 1]->update(outputs[i].getValue());//filtered
		else plot[2 * i + 1]->update(_input);//source

		if (i == index) input = _input;
	}

	//output
	if (enableSmooth) {
		if (bNormalized) output = outputs[index].getValueN();
		else output = outputs[index].getValue();
	}

	//bypass
	if (!enableSmooth)
	{
		output = input;
	}

	//--

	// bangs
	for (int i = 0; i < NUM_VARS; i++) {
		//if (i!=0)continue;

		if (outputs[i].getTrigger()) {
			if (i == index) ofLogNotice() << "Trigger: " << i;
		}

		if (outputs[i].getBonk()) {
			if (i == index) ofLogNotice() << "Bonk: " << i;
		}

		// if the direction has changed and
		// if the time of change is greater than 0.5 sec
		// print the time between changes and amount of change
		if (outputs[i].getDirectionTimeDiff() > 0.5 &&
			outputs[i].directionHasChanged()) {
			if (i == index) ofLogNotice() <<
				"Direction: " << i << " " << outputs[i].getDirectionTimeDiff() <<
				" " << outputs[i].getDirectionValDiff();
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofClear(colorBg);

	//drawPlots(ofGetCurrentViewport());
	drawPlots(rectangle_Plots);

	ofSetColor(ofColor(255, 4));
	//ofDrawRectangle(rectangle_Plots);
	rectangle_Plots.draw();

	//-

	//gui.draw();

	draw_ImGui();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == ' ') bTrigManual = !bTrigManual;
	if (key == OF_KEY_RETURN) bModeNoise = !bModeNoise;

	if (key == 's') solo = !solo;
	if (key == OF_KEY_UP) {
		index--;
		index = ofClamp(index, index.getMin(), index.getMax());
	}
	if (key == OF_KEY_DOWN) {
		index++;
		index = ofClamp(index, index.getMin(), index.getMax());
	}

	if (key == '-') {
		threshold = threshold.get() - 0.05f;
		threshold = ofClamp(threshold, threshold.getMin(), threshold.getMax());
	}
	if (key == '+') {
		threshold = threshold.get() + 0.05f;
		threshold = ofClamp(threshold, threshold.getMin(), threshold.getMax());
	}
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
	params.add(index.set("index", 0, 0, 4));
	params.add(enable.set("enable", true));
	params.add(solo.set("solo", false));
	params.add(minInput.set("minInput", 0, _inputMinRange, _inputMaxRange));
	params.add(maxInput.set("maxInput", 1, _inputMinRange, _inputMaxRange));
	params.add(bNormalized.set("bNormalized", false));
	params.add(minOutput.set("minOutput", 0, _outMinRange, _outMaxRange));
	params.add(maxOutput.set("maxOutput", 1, _outMinRange, _outMaxRange));
	params.add(enableSmooth.set("smooth", true));
	params.add(type.set("type", 0, 0, 2));
	params.add(type_Str.set(" ", ""));
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


	typeLabels.push_back("None");
	typeLabels.push_back("Accumulator");
	typeLabels.push_back("Slide");

	//exclude
	input.setSerializable(false);
	output.setSerializable(false);
	solo.setSerializable(false);
	type_Str.setSerializable(false);
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

		if (name == threshold.getName())
		{
			for (int i = 0; i < NUM_VARS; i++) {
				outputs[i].setThresh(threshold);
			}
		}

		if (name == minOutput.getName() || name == maxOutput.getName())
		{
			for (int i = 0; i < NUM_VARS; i++) {
				outputs[i].setOutputRange(ofVec2f(minOutput, maxOutput));
			}
		}

		if (name == bNormalized.getName())
		{
			for (int i = 0; i < NUM_VARS; i++) {
				//                outputs[i].setOutputRange(ofVec2f(minOutput, maxOutput));

				if (bNormalized) outputs[i].setNormalized(bNormalized, ofVec2f(0, 1));
				else outputs[i].setNormalized(bNormalized, ofVec2f(minOutput, maxOutput));
			}
		}

		if (name == smoothVal.getName())
		{
			int MAX_HISTORY = 30;
			float v = ofMap(smoothVal, 0, 1, 1, MAX_HISTORY);
			for (int i = 0; i < NUM_VARS; i++) {
				outputs[i].initAccum(v);
			}
		}

		if (name == slideMin.getName() || name == slideMax.getName())
		{
			for (int i = 0; i < NUM_VARS; i++) {
				const int MIN_SLIDE = 1;
				const int MAX_SLIDE = 50;
				float _slmin = ofMap(slideMin, 0, 1, MIN_SLIDE, MAX_SLIDE, true);
				float _slmax = ofMap(slideMax, 0, 1, MIN_SLIDE, MAX_SLIDE, true);

				outputs[i].initSlide(_slmin, _slmax);
			}
		}

		//detect "bonks" (onsets):
		//amp.setBonk(0.1, 0.1);  // min growth for onset, min decay
		//set growth/decay:
		//amp.setDecayGrow(true, 0.99); // a framerate-dependent steady decay/growth
		if (name == onsetGrow.getName() || name == onsetDecay.getName())
		{
			for (int i = 0; i < NUM_VARS; i++) {
				outputs[i].setBonk(onsetGrow, onsetDecay);
				//specAmps[i].setDecayGrow(true, 0.99);
			}
		}

		if (name == type.getName())
		{
			switch (type)
			{
			case ofxDataStream::SMOOTHING_NONE:
			{
				type_Str = typeLabels[0];
				if (enableSmooth) enableSmooth = false;
			}
			break;

			case ofxDataStream::SMOOTHING_ACCUM:
			{
				if (!enableSmooth) enableSmooth = true;
				int MAX_HISTORY = 30;
				float v = ofMap(smoothVal, 0, 1, 1, MAX_HISTORY);
				for (int i = 0; i < NUM_VARS; i++) {
					outputs[i].initAccum(v);
				}
				type_Str = typeLabels[1];
			}
			break;

			case ofxDataStream::SMOOTHING_SLIDE:
			{
				if (!enableSmooth) enableSmooth = true;
				type_Str = typeLabels[2];
				for (int i = 0; i < NUM_VARS; i++) {
					const int MIN_SLIDE = 1;
					const int MAX_SLIDE = 50;
					float _slmin = ofMap(slideMin, 0, 1, MIN_SLIDE, MAX_SLIDE, true);
					float _slmax = ofMap(slideMax, 0, 1, MIN_SLIDE, MAX_SLIDE, true);

					outputs[i].initSlide(_slmin, _slmax);
				}
			}
			break;
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::setup_ImGui()
{
	ImGuiConfigFlags flags = ImGuiConfigFlags_DockingEnable;
	bool bAutoDraw = false;
	bool bRestore = true;
	bool bMouse = false;
	gui.setup(nullptr, bAutoDraw, flags, bRestore, bMouse);

	auto &io = ImGui::GetIO();
	auto normalCharRanges = io.Fonts->GetGlyphRangesDefault();

	//-

	// font
	std::string fontName;
	float fontSizeParam;
	fontName = "telegrama_render.otf";
	fontSizeParam = 11;

	std::string _path = "assets/fonts/"; // assets folder
	customFont = gui.addFont(_path + fontName, fontSizeParam, nullptr, normalCharRanges);
	io.FontDefault = customFont;

	//-

	// theme
	ofxSurfingHelpers::ImGui_ThemeMoebiusSurfing();
	//ofxSurfingHelpers::ImGui_ThemeModernDark();
}

//--------------------------------------------------------------
void ofApp::draw_ImGui()
{
	gui.begin();
	{
		bLockMouseByImGui = false;

		//panels sizes
		float xx = 10;
		float yy = 10;
		float ww = PANEL_WIDGETS_WIDTH;
		float hh = PANEL_WIDGETS_HEIGHT;

		//widgets sizes
		float _spcx;
		float _spcy;
		float _w100;
		float _h100;
		float _w99;
		float _w50;
		float _w33;
		float _w25;
		float _h;
		ofxSurfingHelpers::refreshImGui_WidgetsSizes(_spcx, _spcy, _w100, _h100, _w99, _w50, _w33, _w25, _h);

		mainSettings = ofxImGui::Settings();

		ImGuiWindowFlags flagsw = auto_resize ? ImGuiWindowFlags_AlwaysAutoResize : ImGuiWindowFlags_None;
		//flagsw |= ImGuiCond_FirstUseEver;

		//if (auto_lockToBorder) flagsw |= ImGuiCond_Always;
		//else flagsw |= ImGuiCond_FirstUseEver;

		//ImGui::SetNextWindowSize(ImVec2(ww, hh), flagsw);
		//ImGui::SetNextWindowPos(ImVec2(xx, yy), flagsw);

		ImGui::PushFont(customFont);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(ww, hh));
		{
			std::string n = "DATA STREAM";
			if (ofxImGui::BeginWindow(n.c_str(), mainSettings, flagsw))
			{
				ofxSurfingHelpers::refreshImGui_WidgetsSizes(_spcx, _spcy, _w100, _h100, _w99, _w50, _w33, _w25, _h);

				ofxImGui::AddCombo(type, typeLabels);

				ofxImGui::AddGroup(params, mainSettings);

				//ofxSurfingHelpers::AddPlot(input);
				//ofxSurfingHelpers::AddPlot(output);

				//ImGui::PushItemWidth(-100);
				//ofxImGui::AddParameter(_param);
				//ImGui::PopItemWidth();
				//if (ImGui::Button("_Button", ImVec2(_w100, _h / 2))) {}
				//ofxSurfingHelpers::AddBigToggle(_param, _w100, _h);
				//ImGui::PushButtonRepeat(true);
				//float __w = ofxSurfingHelpers::getImGui_WidgetWidth(w, 2);
				//if (ImGui::Button("<", ImVec2(__w, _h))) {} ImGui::SameLine();
				//if (ImGui::Button(">", ImVec2(__w, _h))) {}
				//ImGui::PopButtonRepeat();

				//--

				//mouse lockers

				bLockMouseByImGui = bLockMouseByImGui | ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
				bLockMouseByImGui = bLockMouseByImGui | ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
				bLockMouseByImGui = bLockMouseByImGui | ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
				
				ImGui::Dummy(ImVec2(0.0f, 2.0f));
				if (ImGui::CollapsingHeader("ADVANCED"))
				{
					ofxImGui::AddParameter(rectangle_Plots.bEditMode);
				ImGui::Dummy(ImVec2(0.0f, 2.0f));
					ofxImGui::AddParameter(auto_resize);
					ofxImGui::AddParameter(bLockMouseByImGui);
					//ofxImGui::AddParameter(auto_lockToBorder);
				}
			}
			ofxImGui::EndWindow(mainSettings);
		}
		ImGui::PopStyleVar();
		ImGui::PopFont();
	}
	gui.end();

	gui.draw();
}