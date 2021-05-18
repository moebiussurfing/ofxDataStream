#include "DataStreamGroup.h"

//--------------------------------------------------------------
void DataStreamGroup::drawPlots(ofRectangle r) {

	ofPushStyle();

	int hh = r.getHeight();
	int ww = r.getWidth();
	int x = r.getX();
	int y = r.getY();

	ofColor colorBaseLine = ofColor(255, 48);

	//ofPushMatrix();
	//ofTranslate(x,y)

	int h;
	if (solo) {
		h = hh / 2;//full height on solo
		y = y - (hh * index);
	}
	else {
		h = hh / NUM_PLOTS;
	}

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

		//mark selected
		if (i == index * 2 || i == index * 2 + 1)
		{
			ofSetLineWidth(1);
			ofSetColor(255, 100);
			ofLine(x, y, x, y + h);
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
		ofSetColor(colorBaseLine);
		ofSetLineWidth(1);
		ofLine(x, y + h, x + ww, y + h);

		//name
		ofSetColor(255, 32);
		ofDrawBitmapString(/*"name: " + */ofToString(i), x + 5, y + 11);

		y += h;
	}

	//--

	//if (solo) {
	//	h = hh / 2;//full height on solo
	//	//y = y - (hh * index);

	//	//baseline
	//	ofSetColor(colorBaseLine);
	//	ofSetLineWidth(1);
	//	ofLine(x, y + h, x + ww, y + h);
	//	ofLine(x, y + 2 * h, x + ww, y + 2 * h);
	//}

	//ofPopMatrix();
	ofPopStyle();
}

//--------------------------------------------------------------
void DataStreamGroup::setupPlots() {

	//rectangle_Plots.setAutoSave(true);
	//rectangle_Plots.enableEdit();
	//rectangle_Plots.setRect(10, 10, 400, 400);

	int _size = mParamsGroup.size();

	for (int i = 0; i < NUM_PLOTS; i++)
		//for (int i = 0; i < _size; i++) 
	{
		string _name;
		string _name2;
		_name2 = ofToString(i / 2);
		//_name2 = mParamsGroup[i].getName();
		if (i % 2 == 0) _name = "Input" + _name2;
		else _name = "Output" + _name2;

		plot[i] = new ofxHistoryPlot(NULL, _name, 200, false);
		plot[i]->setRange(0, 1);
		plot[i]->setColor(colors[i]);
		plot[i]->setShowNumericalInfo(false);
		plot[i]->setShowSmoothedCurve(false);

		//int h = ofGetHeight() / NUM_PLOTS;
		//plot[i]->setGridUnit(h / 4);
		//plot[i]->setDrawBackground(false);
		plot[i]->setDrawGrid(false);
		plot[i]->setDrawTitle(true);
	}
}

//--------------------------------------------------------------
void DataStreamGroup::setup() {
	setupParams();

	//gui.setup(params);
	//gui.setPosition(80, 80);

	ofBackground(66);
	ofEnableAlphaBlending();
	ofSetFrameRate(25);

	colorBg = ofColor(32);

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

	//--

	setupPlots();

	//--

	inputs.resize(NUM_VARS);

	outputs.resize(NUM_VARS);
	for (int i = 0; i < NUM_VARS; i++)
	{
		//default init
		outputs[i].initAccum(100);
		outputs[i].directionChangeCalculated = true;
		outputs[i].setBonk(0.1, 0.0);
	}

	//--

	setup_ImGui();

	doReset();

	//--

	//startup
	ofxSurfingHelpers::loadGroup(params, path_Settings);

	//--

	mParamsGroup.setName("ofxMidiParams");
	ofAddListener(mParamsGroup.parameterChangedE(), this, &DataStreamGroup::Changed_Controls_Out);
}

//--------------------------------------------------------------
void DataStreamGroup::update() {
	//updateGenerators();
	updateSmooths();
	updatePlots();
}

//--------------------------------------------------------------
void DataStreamGroup::updateSmooths() {

	for (int i = 0; i < mParamsGroup.size(); i++) {
		//ofLogNotice() << __FUNCTION__ << " mParamsGroup.size() : " << mParamsGroup.size();
		ofAbstractParameter& aparam = mParamsGroup[i];

		string str = "";
		string name = aparam.getName();
		float value = 0;

		if (aparam.type() == typeid(ofParameter<int>).name()) {
			ofParameter<int> ti = aparam.cast<int>();
			value = ofMap(ti, ti.getMin(), ti.getMax(), 0, 1);
			//ofLogNotice() << __FUNCTION__ << " " << ti.getName() << " : " << ti.get() << " : " << value;
		}
		else if (aparam.type() == typeid(ofParameter<float>).name()) {
			ofParameter<float> ti = aparam.cast<float>();
			value = ofMap(ti, ti.getMin(), ti.getMax(), 0, 1);
			//ofLogNotice() << __FUNCTION__ << " " << ti.getName() << " : " << ti.get() << " : " << value;
		}
		else if (aparam.type() == typeid(ofParameter<bool>).name()) {
			ofParameter<bool> ti = aparam.cast<bool>();
			value = ofMap(ti, ti.getMin(), ti.getMax(), 0, 1);
			//ofLogNotice() << __FUNCTION__ << " " << ti.getName() << " : " << ti.get() << " : " << value;
		}
		else {
			continue;
		}

		inputs[i] = value;

		outputs[i].update(inputs[i]); // raw value, index (optional)
	}
	//ofLogNotice() << __FUNCTION__ << "--------------------------------------------------------------";

	//---
}

//--------------------------------------------------------------
void DataStreamGroup::updatePlots() {

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
void DataStreamGroup::updateGenerators() {

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
}

//--------------------------------------------------------------
void DataStreamGroup::draw() {
	ofClear(colorBg);

	if (bShowPlots) {
		//drawPlots(ofGetCurrentViewport());
		drawPlots(rectangle_Plots);

		ofSetColor(ofColor(255, 4));
		//ofDrawRectangle(rectangle_Plots);
		rectangle_Plots.draw();
	}

	//-

	//gui.draw();

	draw_ImGui();
}

//--------------------------------------------------------------
void DataStreamGroup::keyPressed(int key) {
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
void DataStreamGroup::keyReleased(int key) {

}

//--------------------------------------------------------------
void DataStreamGroup::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void DataStreamGroup::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void DataStreamGroup::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void DataStreamGroup::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void DataStreamGroup::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void DataStreamGroup::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void DataStreamGroup::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void DataStreamGroup::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void DataStreamGroup::dragEvent(ofDragInfo dragInfo) {

}

//--------------------------------------------------------------
void DataStreamGroup::setupParams() {

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
	//        _outMinRange = 0;
	//        _outMaxRange = 127;
	//    }

	params.setName(name);
	params.add(index.set("index", 0, 0, 4));//TODO: 5 channels / generators
	params.add(enable.set("ENABLE", true));
	params.add(bShowPlots.set("Show Plots", true));
	params.add(enableSmooth.set("SMOOTH", true));
	params.add(solo.set("SOLO", false));
	params.add(minInput.set("min Input", 0, _inputMinRange, _inputMaxRange));
	params.add(maxInput.set("max Input", 1, _inputMinRange, _inputMaxRange));
	params.add(minOutput.set("min Output", 0, _outMinRange, _outMaxRange));
	params.add(maxOutput.set("max Output", 1, _outMinRange, _outMaxRange));
	params.add(bNormalized.set("Normalized", false));
	params.add(typeSmooth.set("Type Smooth", 0, 0, 2));
	params.add(typeSmooth_Str.set(" ", ""));
	params.add(typeMean.set("Type Mean", 0, 0, 2));
	params.add(typeMean_Str.set(" ", ""));
	params.add(smoothVal.set("Smooth Power", 0.25, 0.0, 1));
	params.add(slideMin.set("Slide In", 0.2, 0.0, 1));
	params.add(slideMax.set("Slide Out", 0.2, 0.0, 1));
	params.add(onsetGrow.set("Onset Grow", 0.1, 0.0, 1));
	params.add(onsetDecay.set("Onset Decay", 0.1, 0.0, 1));
	params.add(threshold.set("Threshold", 0.5, 0.0, 1));
	params.add(bReset.set("RESET", false));
	//params.add(bClamp.set("CLAMP", true));

	params.add(input.set("INPUT", 0, _inputMinRange, _inputMaxRange));
	params.add(output.set("OUTPUT", 0, _outMinRange, _outMaxRange));

	typeSmoothLabels.clear();
	typeSmoothLabels.push_back("None");
	typeSmoothLabels.push_back("Accumulator");
	typeSmoothLabels.push_back("Slide");

	typeMeanLabels.clear();
	typeMeanLabels.push_back("Arith");
	typeMeanLabels.push_back("Geom");
	typeMeanLabels.push_back("Harm");

	//exclude
	input.setSerializable(false);
	output.setSerializable(false);
	solo.setSerializable(false);
	typeSmooth_Str.setSerializable(false);
	typeMean_Str.setSerializable(false);
	bReset.setSerializable(false);

	ofAddListener(params.parameterChangedE(), this, &DataStreamGroup::Changed_params); // setup()
}

//--------------------------------------------------------------
void DataStreamGroup::exit() {
	ofRemoveListener(params.parameterChangedE(), this, &DataStreamGroup::Changed_params); // exit()

	ofxSurfingHelpers::saveGroup(params, path_Settings);
}

//--------------------------------------------------------------
void DataStreamGroup::doReset() {
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
	smoothVal = 0.5;
	typeSmooth = 1;
	typeMean = 0;
	bClamp = true;
}

// callback for a parameter group
//--------------------------------------------------------------
void DataStreamGroup::Changed_params(ofAbstractParameter &e)
{
	//if (!DISABLE_Callbacks)
	{
		string name = e.getName();
		if (name != input.getName() && name != output.getName())
		{
			ofLogNotice() << __FUNCTION__ << " : " << name << " : with value " << e;
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
				//outputs[i].setOutputRange(ofVec2f(minOutput, maxOutput));

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

			if (typeSmooth != ofxDataStream::SMOOTHING_ACCUM) typeSmooth = ofxDataStream::SMOOTHING_ACCUM;
		}

		if (name == enableSmooth.getName())
		{
			if (!typeSmooth) typeSmooth = 1;
		}

		if (name == slideMin.getName() || name == slideMax.getName())
		{
			for (int i = 0; i < NUM_VARS; i++) {
				const int MIN_SLIDE = 1;
				const int MAX_SLIDE = 50;
				float _slmin = ofMap(slideMin, 0, 1, MIN_SLIDE, MAX_SLIDE, true);
				float _slmax = ofMap(slideMax, 0, 1, MIN_SLIDE, MAX_SLIDE, true);

				outputs[i].initSlide(_slmin, _slmax);

				if (typeSmooth != ofxDataStream::SMOOTHING_SLIDE) typeSmooth = ofxDataStream::SMOOTHING_SLIDE;
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

		if (name == typeSmooth.getName())
		{
			switch (typeSmooth)
			{
			case ofxDataStream::SMOOTHING_NONE:
			{
				if (!enableSmooth) enableSmooth = false;
				typeSmooth_Str = typeSmoothLabels[0];
			}
			break;

			case ofxDataStream::SMOOTHING_ACCUM:
			{
				if (!enableSmooth) enableSmooth = true;
				typeSmooth_Str = typeSmoothLabels[1];
				int MAX_HISTORY = 30;
				float v = ofMap(smoothVal, 0, 1, 1, MAX_HISTORY);
				for (int i = 0; i < NUM_VARS; i++) {
					outputs[i].initAccum(v);
				}
			}
			break;

			case ofxDataStream::SMOOTHING_SLIDE:
			{
				if (!enableSmooth) enableSmooth = true;
				typeSmooth_Str = typeSmoothLabels[2];
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

		//-

		if (name == typeMean.getName())
		{
			switch (typeMean)
			{
			case ofxDataStream::MEAN_ARITH:
			{
				typeMean_Str = typeMeanLabels[0];
				for (int i = 0; i < NUM_VARS; i++) {
					outputs[i].setMeanType(ofxDataStream::MEAN_ARITH);
				}
			}
			break;

			case ofxDataStream::MEAN_GEOM:
			{
				typeMean_Str = typeMeanLabels[1];
				for (int i = 0; i < NUM_VARS; i++) {
					outputs[i].setMeanType(ofxDataStream::MEAN_GEOM);
				}
			}
			break;

			case ofxDataStream::MEAN_HARM:
			{
				typeMean_Str = typeMeanLabels[2];
				for (int i = 0; i < NUM_VARS; i++) {
					outputs[i].setMeanType(ofxDataStream::MEAN_HARM);
				}
			}
			break;
			}
		}
	}
}

//--------------------------------------------------------------
void DataStreamGroup::setup_ImGui()
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
void DataStreamGroup::draw_ImGui()
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
		float _h50 = _h / 2;

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

				ImGui::Dummy(ImVec2(0.0f, 2.0f));

				//ofxImGui::AddGroup(params, mainSettings);// group

				//ofxImGui::AddParameter(index);
				if (ofxImGui::AddStepper(index)) {
					index = ofClamp(index, index.getMin(), index.getMax());
				}
				ofxSurfingHelpers::AddBigToggle(enable, _w100, _h);
				ofxSurfingHelpers::AddBigToggle(solo, _w100, _h50);

				bool bOpen = false;
				ImGuiColorEditFlags _flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
				if (ImGui::TreeNodeEx("Clamp", _flagt))
				{
					ofxSurfingHelpers::refreshImGui_WidgetsSizes(_spcx, _spcy, _w100, _h100, _w99, _w50, _w33, _w25, _h);
					ofxImGui::AddParameter(minInput);
					ofxImGui::AddParameter(maxInput);
					ofxImGui::AddParameter(minOutput);
					ofxImGui::AddParameter(maxOutput);
					ofxSurfingHelpers::AddBigToggle(bNormalized, _w100, _h50);
					ImGui::TreePop();
				}
				ImGui::Dummy(ImVec2(0.0f, 2.0f));

				ofxImGui::AddParameter(input);
				ofxImGui::AddParameter(output);
				ImGui::Dummy(ImVec2(0.0f, 2.0f));

				ofxSurfingHelpers::AddBigToggle(enableSmooth, _w100, _h);
				if (enableSmooth) {
					if (typeSmooth == ofxDataStream::SMOOTHING_ACCUM) {
						ofxImGui::AddParameter(smoothVal);
					}
					if (typeSmooth == ofxDataStream::SMOOTHING_SLIDE)
					{
						ofxImGui::AddParameter(slideMin);
						ofxImGui::AddParameter(slideMax);
					}
					ofxImGui::AddCombo(typeSmooth, typeSmoothLabels);
					ofxImGui::AddCombo(typeMean, typeMeanLabels);
					ImGui::Dummy(ImVec2(0.0f, 2.0f));

					if (ImGui::TreeNode("OnSets"))
					{
						ofxSurfingHelpers::refreshImGui_WidgetsSizes(_spcx, _spcy, _w100, _h100, _w99, _w50, _w33, _w25, _h);
						ofxImGui::AddParameter(onsetGrow);
						ofxImGui::AddParameter(onsetDecay);
						ImGui::TreePop();
					}
					ofxImGui::AddParameter(threshold);
				}
				ImGui::Dummy(ImVec2(0.0f, 2.0f));
				ofxSurfingHelpers::AddBigToggle(bReset, _w100, _h50);

				//plots
				//ofxSurfingHelpers::AddPlot(input);
				//ofxSurfingHelpers::AddPlot(output);

				/*
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
				*/

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

			//----

			//window

			flagsw |= ImGuiWindowFlags_NoCollapse;

			string name;

			name = "INPUTS";
			if (ofxImGui::BeginWindow(name.c_str(), mainSettings, flagsw))
			{
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

				ofxImGui::AddGroup(mParamsGroup, mainSettings);
			}
			ofxImGui::EndWindow(mainSettings);


			name = "OUTPUTS";
			if (ofxImGui::BeginWindow(name.c_str(), mainSettings, flagsw))
			{
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

				for (int i = 0; i < mParamsGroup.size(); i++)
				{
					auto type = mParamsGroup[i].type();
					bool isGroup = type == typeid(ofParameterGroup).name();
					bool isFloat = type == typeid(ofParameter<float>).name();
					bool isInt = type == typeid(ofParameter<int>).name();
					bool isBool = type == typeid(ofParameter<bool>).name();
					string str = mParamsGroup[i].getName();

					if (isFloat)
					{
						float v = outputs[i].getValue();
						float min = mParamsGroup[i].cast<float>().getMin();
						float max = mParamsGroup[i].cast<float>().getMax();
						v = ofMap(v, 0, 1, min, max);
						ImGui::SliderFloat(str.c_str(), &v, min, max);
					}
					else if (isInt)
					{
						float vf = outputs[i].getValue();
						int vi;
						int min = mParamsGroup[i].cast<int>().getMin();
						int max = mParamsGroup[i].cast<int>().getMax();
						vi = (int)ofMap(vf, 0, 1, min, max);
						ImGui::SliderInt(str.c_str(), &vi, min, max);
					}
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

//--------------------------------------------------------------
void DataStreamGroup::addParam(ofAbstractParameter& aparam) {
	auto mac = make_shared<DataStreamGroup::MidiParamAssoc>();
	mac->paramIndex = mParamsGroup.size();

	//ofLogWarning() << __FUNCTION__ << " ";

	if (aparam.type() == typeid(ofParameter<int>).name()) {
		mac->ptype = PTYPE_INT;
		ofParameter<int> ti = aparam.cast<int>();
		ofParameterGroup pgroup = ti.getFirstParent();
		if (pgroup) {
			mac->xmlParentName = pgroup.getEscapedName();
		}
	}
	else if (aparam.type() == typeid(ofParameter<float>).name()) {
		mac->ptype = PTYPE_FLOAT;
		ofParameter<float> fi = aparam.cast<float>();
		ofParameterGroup pgroup = fi.getFirstParent();
		if (pgroup) {
			mac->xmlParentName = pgroup.getEscapedName();
		}
	}
	else if (aparam.type() == typeid(ofParameter<bool>).name()) {
		mac->ptype = PTYPE_BOOL;
		ofParameter<bool> bi = aparam.cast<bool>();
		ofParameterGroup pgroup = bi.getFirstParent();
		if (pgroup) {
			mac->xmlParentName = pgroup.getEscapedName();
		}
	}
	if (mac->ptype == PTYPE_UNKNOWN) {
		//ofLogNotice("ofxMidiParams :: addParam : unsupported param type");
		return;
	}

	mac->xmlName = aparam.getEscapedName();

	mParamsGroup.add(aparam);
	mAssocParams.push_back(mac);
	//_updatePositions();

	//-

	////TODO:
	//ofRemoveListener(mParamsGroup.parameterChangedE(), this, &ofxMidiParams::Changed_Controls_Out);
	//ofAddListener(mParamsGroup.parameterChangedE(), this, &ofxMidiParams::Changed_Controls_Out);

	//-

	//startup();
}

//--------------------------------------------------------------
void DataStreamGroup::addGroup(ofParameterGroup aparams) {
	for (int i = 0; i < aparams.size(); i++) {
		addParam(aparams.get(i));
	}

	//--

	//int _size = mParamsGroup.size();
	//outputs.resize(_size);
	//inputs.resize(_size);
	//for (int i = 0; i < _size; i++)
	//{
	//	//default init
	//	outputs[i].initAccum(100);
	//	outputs[i].directionChangeCalculated = true;
	//	outputs[i].setBonk(0.1, 0.0);
	//}

	//setupPlots();


	//setupPlots();

	//inputs.resize(NUM_VARS);

	//outputs.resize(NUM_VARS);
	//for (int i = 0; i < NUM_VARS; i++)
	//{
	//	//default init
	//	outputs[i].initAccum(100);
	//	outputs[i].directionChangeCalculated = true;
	//	outputs[i].setBonk(0.1, 0.0);
	//}
}

//--------------------------------------------------------------
void DataStreamGroup::add(ofParameterGroup aparams) {
	for (int i = 0; i < aparams.size(); i++) {
		addParam(aparams.get(i));
	}
}

//--------------------------------------------------------------
void DataStreamGroup::add(ofParameter<float>& aparam) {
	addParam(aparam);
}

//--------------------------------------------------------------
void DataStreamGroup::add(ofParameter<bool>& aparam) {
	addParam(aparam);
}

//--------------------------------------------------------------
void DataStreamGroup::add(ofParameter<int>& aparam) {
	addParam(aparam);
}

//--------------------------------------------------------------
void DataStreamGroup::Changed_Controls_Out(ofAbstractParameter &e)
{
	std::string name = e.getName();

	ofLogNotice(__FUNCTION__) << name << " : " << e;

}
