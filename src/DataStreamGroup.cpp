#include "DataStreamGroup.h"

//--------------------------------------------------------------
void DataStreamGroup::doRandomize() {
	for (int i = 0; i < mParamsGroup.size(); i++) {
		auto &p = mParamsGroup[i];

		if (p.type() == typeid(ofParameter<float>).name()) {
			ofParameter<float> pr = p.cast<float>();
			pr = ofRandom(pr.getMin(), pr.getMax());
		}
		else if (p.type() == typeid(ofParameter<int>).name()) {
			ofParameter<int> pr = p.cast<int>();
			pr = ofRandom(pr.getMin(), pr.getMax());
		}
	}
}

//--------------------------------------------------------------
void DataStreamGroup::drawPlots(ofRectangle r) {

	ofPushStyle();

	int hh = r.getHeight();
	int ww = r.getWidth();
	int x = r.getX();
	int y = r.getY();

	colorBaseLine = ofColor(255, 48);
	colorSelected = ofColor(255, 150);

	//ofPushMatrix();
	//ofTranslate(x,y)

	int h;
	if (!solo)
	{
		h = hh / NUM_VARS;
	}
	else // solo
	{
		h = hh; // full height on solo
	}

	for (int i = 0; i < NUM_VARS; i++)
	{
		if (solo) if (i != index) continue;

		int ii = 2 * i;

		////grid
		//int hg = h / 2;
		//plot[ii]->setGridUnit(hg);
		//plot[ii + 1]->setGridUnit(hg);

		plots[ii]->draw(x, y, ww, h);
		plots[ii + 1]->draw(x, y, ww, h);

		//baseline
		ofSetColor(colorBaseLine);
		ofSetLineWidth(1);
		ofLine(x, y + h, x + ww, y + h);

		//name
		if (i == index) ofSetColor(colorSelected);
		else ofSetColor(colorBaseLine);
		string s = "#" + ofToString(i);
		ofDrawBitmapString(s, x + 5, y + 11);

		ofColor _c1 = colorSelected;//white
		ofColor _c2 = colorSelected;
		//ofColor _c1 = ofColor(colors[ii]);//colored
		//ofColor _c2 = ofColor(colors[ii]);

		float _a1 = ofxSurfingHelpers::Bounce(1.0);
		float _a2 = ofxSurfingHelpers::Bounce(0.5);

		//threshold
		{
			ofColor c;
			if (outputs[i].getBonk()) c.set(ofColor(_c2, 90 * _a1));//bonked
			else if (outputs[i].getTrigger()) c.set(ofColor(_c2, 140 * _a2));//trigged
			else c.set(ofColor(_c2, 20));//standby
			ofSetColor(c);
			float yth = y + (1 - threshold)*h;
			ofLine(x, yth, x + ww, yth);
		}

		//mark selected
		if (i == index && !solo)
		{
			ofSetLineWidth(1);
			ofSetColor(colorSelected);
			ofLine(x, y, x, y + h);
		}

		if (!solo) y += h;
	}

	//ofPopMatrix();
	ofPopStyle();
}

//--------------------------------------------------------------
void DataStreamGroup::setupPlots() {
	NUM_VARS = mParamsGroup.size();
	NUM_PLOTS = 2 * NUM_VARS;

	index.setMax(NUM_VARS - 1);

	plots.resize(NUM_PLOTS);

	//colors
	colors.clear();
	colors.resize(NUM_PLOTS);

	//alphas
	int a1 = 48;//input
	int a2 = 255;//output
	ofColor c;

#ifdef COLORS_MONCHROME
	for (int i = 0; i < NUM_VARS; i++)
	{
		c = colorPlots;
		colors[2 * i] = ofColor(c, a1);
		colors[2 * i + 1] = ofColor(c, a2);
	}
#endif

#ifndef COLORS_MONCHROME
	int sat = 255;
	int brg = 255;
	int hueStep = 255. / (float)NUM_VARS;
	for (int i = 0; i < NUM_VARS; i++)
	{
		c.setHsb(hueStep * i, sat, brg);
		colors[2 * i] = ofColor(c, a1);
		colors[2 * i + 1] = ofColor(c, a2);
	}
#endif

	for (int i = 0; i < NUM_PLOTS; i++)
	{
		string _name;
		string _name2;
		_name2 = ofToString(mParamsGroup[i / 2].getName());//param name
		//_name2 = ofToString(i / 2);//index as name

		bool b1 = (i % 2 == 0);//1st plot of each var. input
		_name = _name2;
		//if (b1) _name = "Input " + _name2;
		//else _name = "Output " + _name2;

		bool bTitle = !b1;
		bool bInfo = false;
		bool bBg = b1;
		bool bGrid = false && b1;

		plots[i] = new ofxHistoryPlot(NULL, _name, 60 * 4, false);//4 secs at 60fps
		plots[i]->setRange(0, 1);
		plots[i]->setColor(colors[i]);
		plots[i]->setDrawTitle(bTitle);
		plots[i]->setShowNumericalInfo(bInfo);
		plots[i]->setShowSmoothedCurve(false);
		plots[i]->setDrawBackground(bBg);
		plots[i]->setDrawGrid(bGrid);
	}

	// draggable rectangle
	ofColor c0(0, 90);
	rectangle_Plots.setColorEditingHover(c0);
	rectangle_Plots.setColorEditingMoving(c0);
}

//--------------------------------------------------------------
void DataStreamGroup::setup() {
	setupParams();

	//--

	generators.resize(NUM_GENERATORS);

	//--

	setup_ImGui();

	//--

	mParamsGroup.setName("ofxMidiParams");
	ofAddListener(mParamsGroup.parameterChangedE(), this, &DataStreamGroup::Changed_Controls_Out);
}

//--------------------------------------------------------------
void DataStreamGroup::startup() {
	bDISABLE_CALLBACKS = false;

	doReset();

	//--

	//startup
	//ofxSurfingHelpers::loadGroup(params, path_Settings);
}


//--------------------------------------------------------------
void DataStreamGroup::update() {
	if (ofGetFrameNum() == 0) { startup(); }

	// play timed randoms
	static int _secs = 2;
	if (bPlay) {
		int max = ofMap(playSpeed, 0, 1, 60, 5) * _secs;
		//int max = 60 * _secs;
		tf = ofGetFrameNum() % max;
		tn = ofMap(tf, 0, max, 0, 1);
		if (tf == 0)
		{
			doRandomize();
		}
	}

	if (bUseGenerators) updateGenerators();
	if (!bUseGenerators) updateSmooths();
	updatePlots();
}

//--------------------------------------------------------------
void DataStreamGroup::updateSmooths() {

	for (int i = 0; i < mParamsGroup.size(); i++) {
		ofAbstractParameter& p = mParamsGroup[i];

		//string str = "";
		//string name = aparam.getName();
		float value = 0;

		if (p.type() == typeid(ofParameter<int>).name()) {
			ofParameter<int> ti = p.cast<int>();
			value = ofMap(ti, ti.getMin(), ti.getMax(), 0, 1);
		}
		else if (p.type() == typeid(ofParameter<float>).name()) {
			ofParameter<float> ti = p.cast<float>();
			value = ofMap(ti, ti.getMin(), ti.getMax(), 0, 1);
		}
		//else if (p.type() == typeid(ofParameter<bool>).name()) {
		//	ofParameter<bool> ti = p.cast<bool>();
		//	value = ofMap(ti, ti.getMin(), ti.getMax(), 0, 1);
		//	//ofLogNotice() << __FUNCTION__ << " " << ti.getName() << " : " << ti.get() << " : " << value;
		//}
		else {
			continue;
		}

		//-

		inputs[i] = value; // prepare and feed input

		outputs[i].update(inputs[i]); // raw value, index (optional)
	}
}

//--------------------------------------------------------------
void DataStreamGroup::updatePlots() {

	for (int i = 0; i < NUM_VARS; i++)
	{
		//input
		float _input = ofClamp(inputs[i], minInput, maxInput);
		plots[2 * i]->update(_input);//source

		//output
		if (enableSmooth) plots[2 * i + 1]->update(outputs[i].getValue());//filtered
		else plots[2 * i + 1]->update(_input);//source

		if (i == index) input = _input;
	}

	//----

	// index selected

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

	//----

	// bangs / onSets

	for (int i = 0; i < NUM_VARS; i++)
	{
		//if (i!=0)continue;

		if (outputs[i].getTrigger()) {
			if (i == index) ofLogVerbose() << "Trigger: " << i;
		}

		if (outputs[i].getBonk()) {
			if (i == index) ofLogVerbose() << "Bonk: " << i;
		}

		// if the direction has changed and
		// if the time of change is greater than 0.5 sec
		// print the time between changes and amount of change
		if (outputs[i].getDirectionTimeDiff() > 0.5 && outputs[i].directionHasChanged())
		{
			if (i == index)
				ofLogVerbose() << "Direction: " << i << " " << outputs[i].getDirectionTimeDiff() << " "
				<< outputs[i].getDirectionValDiff();
		}
	}
}

//--------------------------------------------------------------
void DataStreamGroup::updateGenerators() {
	if (ofGetFrameNum() % 20 == 0) {
		if (ofRandom(0, 1) > 0.5) bTrigManual = !bTrigManual;
	}

	for (int i = 0; i < NUM_GENERATORS; i++) {
		switch (i) {
		case 0: generators[i] = (bTrigManual ? 1 : 0); break;
		case 1: generators[i] = ofxSurfingHelpers::Tick((bModeFast ? 0.2 : 1)); break;
		case 2: generators[i] = ofxSurfingHelpers::Noise(ofPoint((!bModeFast ? 1 : 0.001), (!bModeFast ? 1.3 : 2.3))); break;
		case 3: generators[i] = ofClamp(ofxSurfingHelpers::NextGaussian(0.5, (bModeFast ? 1 : 0.1)), 0, 1); break;
		case 4: generators[i] = ofxSurfingHelpers::NextReal(0, (bModeFast ? 1 : 0.1)); break;
		case 5: generators[i] = ofxSurfingHelpers::Noise(ofPoint((!bModeFast ? 1 : 0.00001), (!bModeFast ? 0.3 : 0.03))); break;
		}

		//outputs[i].update(inputs[i]); // raw value, index (optional)

		//----

		// feed generators to parameters

		for (int i = 0; i < mParamsGroup.size(); i++) {
			ofAbstractParameter& aparam = mParamsGroup[i];

			//string str = "";
			//string name = aparam.getName();
			float value = 0;

			if (aparam.type() == typeid(ofParameter<int>).name()) {
				ofParameter<int> ti = aparam.cast<int>();
				value = ofMap(generators[i], 0, 1, ti.getMin(), ti.getMax());
				ti.set((int)value);
				//ti = (int)value;
			}
			else if (aparam.type() == typeid(ofParameter<float>).name()) {
				ofParameter<float> ti = aparam.cast<float>();
				value = ofMap(generators[i], 0, 1, ti.getMin(), ti.getMax());
				ti.set(value);
				//ti = value;
			}
			//else if (aparam.type() == typeid(ofParameter<bool>).name()) {
			//	ofParameter<bool> ti = aparam.cast<bool>();
			//	value = ofMap(ti, ti.getMin(), ti.getMax(), 0, 1);
			//	//ofLogNotice() << __FUNCTION__ << " " << ti.getName() << " : " << ti.get() << " : " << value;
			//}
			else {
				continue;
			}

			inputs[i] = value; // prepare and feed input
			outputs[i].update(inputs[i]); // raw value, index (optional)
		}
	}
}

//--------------------------------------------------------------
void DataStreamGroup::draw() {
	if (!bShowGui) return;

	if (bShowPlots) {
		ofPushStyle();
		if (bFullScreen) drawPlots(ofGetCurrentViewport());
		else drawPlots(rectangle_Plots);

		ofSetColor(ofColor(255, 4));
		rectangle_Plots.draw();
		ofPopStyle();
	}

	//-

	if (bGui) draw_ImGui();
	//gui.draw();
}

//--------------------------------------------------------------
void DataStreamGroup::keyPressed(int key) {
	if (key == 'g') bGui = !bGui;

	if (key == OF_KEY_RETURN) bTrigManual = !bTrigManual;
	//if (key == OF_KEY_RETURN) bModeNoise = !bModeNoise;

	if (key == ' ') doRandomize();

	if (key == 's') solo = !solo;
	if (key == OF_KEY_UP) {
		index--;
		index = ofClamp(index, index.getMin(), index.getMax());
	}
	if (key == OF_KEY_DOWN) {
		index++;
		index = ofClamp(index, index.getMin(), index.getMax());
	}

	//threshold
	if (key == '-') {
		threshold = threshold.get() - 0.05f;
		threshold = ofClamp(threshold, threshold.getMin(), threshold.getMax());
	}
	if (key == '+') {
		threshold = threshold.get() + 0.05f;
		threshold = ofClamp(threshold, threshold.getMin(), threshold.getMax());
	}

	//types
	if (key == OF_KEY_TAB) {
		if (typeSmooth >= typeSmooth.getMax()) typeSmooth = 1;
		else typeSmooth++;
	}

	if (key == OF_KEY_LEFT_SHIFT) {
		if (typeMean >= typeMean.getMax()) typeMean = 0;
		else typeMean++;
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


	//    if (_bNormalized)//normalizad
	{
		_inputMinRange = 0;
		_inputMaxRange = 1;
		_outMaxRange = 1;
		_outMinRange = 0;
	}
	//    else//midi range
	//    {
	//        _inputMinRange = 0;
	//        _inputMaxRange = 1;
	//        _outMinRange = 0;
	//        _outMaxRange = 127;
	//    }

	params.setName(name);
	params.add(index.set("index", 0, 0, 0));
	params.add(bPlay.set("Play", false));
	params.add(enable.set("ENABLE", true));
	params.add(bShowPlots.set("Show Plots", true));
	params.add(bFullScreen.set("Full Screen", false));
	params.add(bUseGenerators.set("Use Generators", false));
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
	params.add(smoothPower.set("Smooth Power", 0.25, 0.0, 1));
	params.add(slideMin.set("Slide In", 0.2, 0.0, 1));
	params.add(slideMax.set("Slide Out", 0.2, 0.0, 1));
	params.add(onsetGrow.set("Onset Grow", 0.1, 0.0, 1));
	params.add(onsetDecay.set("Onset Decay", 0.1, 0.0, 1));
	params.add(threshold.set("Threshold", 0.85, 0.0, 1));
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

	ofAddListener(params.parameterChangedE(), this, &DataStreamGroup::Changed_Params); // setup()
}

//--------------------------------------------------------------
void DataStreamGroup::exit() {
	ofRemoveListener(params.parameterChangedE(), this, &DataStreamGroup::Changed_Params); // exit()

	ofxSurfingHelpers::saveGroup(params, path_Settings);
}

//--------------------------------------------------------------
void DataStreamGroup::doReset() {
	ofLogNotice(__FUNCTION__) << "output: " << ofToString(output);
	enable = true;
	bPlay = false;
	bUseGenerators = false;
	bShowPlots = true;
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
	smoothPower = 0.5;
	typeSmooth = 1;
	typeMean = 0;
	bClamp = true;
	threshold = 0.85;
}

// callback for a parameter group
//--------------------------------------------------------------
void DataStreamGroup::Changed_Params(ofAbstractParameter &e)
{
	if (bDISABLE_CALLBACKS) return;

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

	if (name == smoothPower.getName())
	{
		int MAX_ACC_HISTORY = 60;//calibrated to 60fps
		float v = ofMap(smoothPower, 0, 1, 1, MAX_ACC_HISTORY);
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

			outputs[i].directionChangeCalculated = true;
			//outputs[i].setBonk(0.1, 0.0);
		}
	}

	if (name == typeSmooth.getName())
	{
		typeSmooth = ofClamp(typeSmooth, typeSmooth.getMin(), typeSmooth.getMax());

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
			float v = ofMap(smoothPower, 0, 1, 1, MAX_HISTORY);
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
		typeMean = ofClamp(typeMean, typeMean.getMin(), typeMean.getMax());

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

//--------------------------------------------------------------
void DataStreamGroup::setup_ImGui()
{
	ImGuiConfigFlags flags = ImGuiConfigFlags_DockingEnable;
	bool bAutoDraw = true;
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
		float hh = 20;
		//float hh = PANEL_WIDGETS_HEIGHT;

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
		//ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(ww, hh));
		{
			std::string n = "DATA STREAM";
			if (ofxImGui::BeginWindow(n.c_str(), mainSettings, flagsw))
			{
				ofxSurfingHelpers::refreshImGui_WidgetsSizes(_spcx, _spcy, _w100, _h100, _w99, _w50, _w33, _w25, _h);

				ImGuiTreeNodeFlags _f;
				_f = ImGuiTreeNodeFlags_None;
				//_f = ImGuiTreeNodeFlags_DefaultOpen;

				if (ImGui::CollapsingHeader("MONITOR", _f))
				{
					//ofxImGui::AddParameter(index);
					if (ofxImGui::AddStepper(index)) {
						index = ofClamp(index, index.getMin(), index.getMax());
					}
					ofxSurfingHelpers::AddBigToggle(solo, _w100, _h50);
					ofxImGui::AddParameter(input);
					ofxImGui::AddParameter(output);
					//ImGui::Dummy(ImVec2(0.0f, 2.0f));
				}
				ImGui::Dummy(ImVec2(0.0f, 2.0f));

				//ofxImGui::AddGroup(params, mainSettings);// group

				//ofxSurfingHelpers::AddBigToggle(enable, _w100, _h);//TODO:

				//TODO:
				//blink by timer
				//tn
				ofxSurfingHelpers::AddBigToggle(bPlay, _w100, _h);
				if (ImGui::Button("RANDOMIZE", ImVec2(_w100, _h50))) {
					doRandomize();
				}
				if (bPlay) {
					ImGui::SliderFloat("Speed", &playSpeed, 0, 1);
				}
				ImGui::Dummy(ImVec2(0.0f, 2.0f));

				ofxSurfingHelpers::AddBigToggle(bShowPlots, _w100, _h50);
				ofxSurfingHelpers::AddBigToggle(bFullScreen, _w100, _h50);
				ofxSurfingHelpers::AddBigToggle(bUseGenerators, _w100, _h50);

				ImGui::Dummy(ImVec2(0.0f, 2.0f));

				ofxSurfingHelpers::AddBigToggle(enableSmooth, _w100, _h);
				if (enableSmooth)
				{
					ofxImGui::AddCombo(typeSmooth, typeSmoothLabels);
					ofxImGui::AddCombo(typeMean, typeMeanLabels);

					if (typeSmooth == ofxDataStream::SMOOTHING_ACCUM) {
						ofxImGui::AddParameter(smoothPower);
					}
					if (typeSmooth == ofxDataStream::SMOOTHING_SLIDE)
					{
						ofxImGui::AddParameter(slideMin);
						ofxImGui::AddParameter(slideMax);
					}
					ImGui::Dummy(ImVec2(0.0f, 2.0f));

					if (ImGui::TreeNode("OnSets"))
					{
						ofxSurfingHelpers::refreshImGui_WidgetsSizes(_spcx, _spcy, _w100, _h100, _w99, _w50, _w33, _w25, _h);
						ofxImGui::AddParameter(onsetGrow);
						ofxImGui::AddParameter(onsetDecay);
						ofxImGui::AddParameter(threshold);
						ImGui::TreePop();
					}
				}
				ImGui::Dummy(ImVec2(0.0f, 2.0f));

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

			//flagsw |= ImGuiWindowFlags_NoCollapse;

			string name;

			name = "INPUTS";
			if (ofxImGui::BeginWindow(name.c_str(), mainSettings, flagsw))
			{
				ofxSurfingHelpers::refreshImGui_WidgetsSizes(_spcx, _spcy, _w100, _h100, _w99, _w50, _w33, _w25, _h);

				ofxImGui::AddGroup(mParamsGroup, mainSettings);
			}
			ofxImGui::EndWindow(mainSettings);


			name = "OUTPUTS";
			if (ofxImGui::BeginWindow(name.c_str(), mainSettings, flagsw))
			{
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
		//ImGui::PopStyleVar();
		ImGui::PopFont();
	}
	gui.end();

	//gui.draw();
}

//----

//--------------------------------------------------------------
void DataStreamGroup::addParam(ofAbstractParameter& aparam) {
	//ofLogNotice() << __FUNCTION__ << " " << aparam.getName();

	//--

	//TODO:
	//nested groups
	// https://forum.openframeworks.cc/t/ofxparametercollection-manage-multiple-ofparameters/34888/3
	auto type = aparam.type();

	//////string typeStr = type;
	bool isGroup = type == typeid(ofParameterGroup).name();
	//bool isFloat = type == typeid(ofParameter<float>).name();
	//bool isInt = type == typeid(ofParameter<int>).name();
	//bool isBool = type == typeid(ofParameter<bool>).name();

	ofLogNotice() << __FUNCTION__ << " " << aparam.getName() << " : " << type;

	if (isGroup)
	{
		auto &g = aparam.castGroup();
		for (int i = 0; i < g.size(); i++) {
			addParam(g.get(i));
		}
	}
	
	// exclude groups to remove from plots
	if(!isGroup) mParamsGroup.add(aparam);

	//--

	//auto mac = make_shared<DataStreamGroup::MidiParamAssoc>();
	//mac->paramIndex = mParamsGroup.size();
	////ofLogWarning() << __FUNCTION__ << " ";
	//if (aparam.type() == typeid(ofParameter<int>).name()) {
	//	mac->ptype = PTYPE_INT;
	//	ofParameter<int> ti = aparam.cast<int>();
	//	ofParameterGroup pgroup = ti.getFirstParent();
	//	if (pgroup) {
	//		mac->xmlParentName = pgroup.getEscapedName();
	//	}
	//}
	//else if (aparam.type() == typeid(ofParameter<float>).name()) {
	//	mac->ptype = PTYPE_FLOAT;
	//	ofParameter<float> fi = aparam.cast<float>();
	//	ofParameterGroup pgroup = fi.getFirstParent();
	//	if (pgroup) {
	//		mac->xmlParentName = pgroup.getEscapedName();
	//	}
	//}
	//else if (aparam.type() == typeid(ofParameter<bool>).name()) {
	//	mac->ptype = PTYPE_BOOL;
	//	ofParameter<bool> bi = aparam.cast<bool>();
	//	ofParameterGroup pgroup = bi.getFirstParent();
	//	if (pgroup) {
	//		mac->xmlParentName = pgroup.getEscapedName();
	//	}
	//}
	//if (mac->ptype == PTYPE_UNKNOWN) {
	//	//ofLogNotice("ofxMidiParams :: addParam : unsupported param type");
	//	return;
	//}
	//mac->xmlName = aparam.getEscapedName();
	//mParamsGroup.add(aparam);
	//mAssocParams.push_back(mac);

	//-

	////TODO:
	//ofRemoveListener(mParamsGroup.parameterChangedE(), this, &ofxMidiParams::Changed_Controls_Out);
	//ofAddListener(mParamsGroup.parameterChangedE(), this, &ofxMidiParams::Changed_Controls_Out);

	//-

	//startup();
}

//--------------------------------------------------------------
void DataStreamGroup::addGroup(ofParameterGroup& aparams) {
	ofLogNotice() << __FUNCTION__ << " " << aparams.getName();

	for (int i = 0; i < aparams.size(); i++) {
		addParam(aparams.get(i));
	}

	//--

	// already added all params content
	// build the smoothers
	// build the plots

	setupPlots();//NUM_VARS is counted here..

	outputs.resize(NUM_VARS);
	inputs.resize(NUM_VARS);

	for (int i = 0; i < NUM_VARS; i++)
	{
		//default init
		outputs[i].initAccum(100);
		outputs[i].directionChangeCalculated = true;
		outputs[i].setBonk(0.1, 0.0);
	}
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
	if (bDISABLE_CALLBACKS) return;

	std::string name = e.getName();

	ofLogVerbose(__FUNCTION__) << name << " : " << e;

}

//--------------------------------------------------------------
ofAbstractParameter& DataStreamGroup::getParam(ofAbstractParameter &e) {
	string name = e.getName();
	auto &p = mParamsGroup.get(name);

	//log
	auto i = mParamsGroup.getPosition(name);
	float value = outputs[i].getValue();
	ofLogVerbose(__FUNCTION__) << name << " : " << value;

	//ofAbstractParameter& aparam = mParamsGroup[i];
	//float value = 0;
	//if (aparam.type() == typeid(ofParameter<int>).name()) {
	//	ofParameter<int> ti = aparam.cast<int>();
	//	value = ofMap(ti, ti.getMin(), ti.getMax(), 0, 1);
	//}
	//else if (aparam.type() == typeid(ofParameter<float>).name()) {
	//	ofParameter<float> ti = aparam.cast<float>();
	//	value = ofMap(ti, ti.getMin(), ti.getMax(), 0, 1);
	//}

	//ofLogNotice(__FUNCTION__) << aparam.getName() << " : " << e;

	return p;
}

//--------------------------------------------------------------
ofAbstractParameter& DataStreamGroup::getParam(string name) {
	//string name = e.getName();
	auto &p = mParamsGroup.get(name);

	//log
	auto i = mParamsGroup.getPosition(name);
	float value = outputs[i].getValue();
	//ofLogNotice(__FUNCTION__) << name << " : " << value;

	return p;
}

//TODO: it seems that intereferes with the param?
//--------------------------------------------------------------
ofParameter<float>& DataStreamGroup::getParamFloat(string name) {
	auto &p = mParamsGroup.get(name);
	auto i = mParamsGroup.getPosition(name);
	if (p.type() == typeid(ofParameter<float>).name()) {
		ofParameter<float> pf = p.cast<float>();
		ofParameter<float> pf_Out = pf;//set min/max
		//ofParameter<float> pf_Out{ pf.getName(), 0, pf.getMin(), pf.getMax() };//set min/max
		float value = ofMap(outputs[i].getValue(), 0, 1, pf.getMin(), pf.getMax());
		pf_Out.set(value);
		//pf.set(outputs[i].getValue());
		return pf_Out;
		//return pf;
	}
	else
	{
		ofParameter<float> pf{ "empty", -1 };
		return pf;
	}
}

//--------------------------------------------------------------
float DataStreamGroup::getParamFloatValue(ofAbstractParameter &e) {
	string name = e.getName();
	auto &p = mParamsGroup.get(name);
	auto i = mParamsGroup.getPosition(name);
	if (p.type() == typeid(ofParameter<float>).name()) {
		ofParameter<float> pf = p.cast<float>();
		return ofMap(outputs[i].getValue(), 0, 1, pf.getMin(), pf.getMax());
	}
	else
	{
		return -1;
	}
}

//--------------------------------------------------------------
int DataStreamGroup::getParamIntValue(ofAbstractParameter &e) {
	string name = e.getName();
	auto &p = mParamsGroup.get(name);
	auto i = mParamsGroup.getPosition(name);
	if (p.type() == typeid(ofParameter<int>).name()) {
		ofParameter<int> pf = p.cast<int>();
		return ofMap(outputs[i].getValue(), 0, 1, pf.getMin(), pf.getMax());
	}
	else
	{
		return -1;
	}
}

//--------------------------------------------------------------
ofParameter<int>& DataStreamGroup::getParamInt(string name) {
	auto &p = mParamsGroup.get(name);
	auto i = mParamsGroup.getPosition(name);
	if (p.type() == typeid(ofParameter<int>).name()) {
		ofParameter<int> pi = p.cast<int>();
		ofParameter<int> pi_Out = pi;//set min/max
		pi.set(outputs[i].getValue());
		return pi_Out;
		//return pi;
	}
	else
	{
		ofParameter<int> pi{ "empty", -1 };
		return pi;
	}
}