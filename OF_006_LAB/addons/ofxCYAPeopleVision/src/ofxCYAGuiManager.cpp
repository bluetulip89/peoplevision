/*
 *  ofxCYAGuiManager.cpp
 *  CYAPeopleVision
 *
 *  Created by Joshua Walton on 12/7/09.
 *  Copyright 2009 Lab at Rockwell Group. All rights reserved.
 *
 */

#include "ofxCYAGuiManager.h"
#include "ofxCYASettings.h"

enum{
	PARAM_INT, PARAM_FLOAT, PARAM_BOOL
};

ofxCYAGuiManager::ofxCYAGuiManager() {
	//JG TODO add drawing event
	ofAddListener(ofEvents.update, this, &ofxCYAGuiManager::update);
	ofAddListener(ofEvents.draw, this, &ofxCYAGuiManager::draw);
	
	ofAddListener(ofEvents.mousePressed, this, &ofxCYAGuiManager::mousePressed);
	ofAddListener(ofEvents.mouseDragged, this, &ofxCYAGuiManager::mouseDragged);
	ofAddListener(ofEvents.mouseReleased, this, &ofxCYAGuiManager::mouseReleased);
	
	enableGui = true;
	//ofxCYASettings *p_Settings;
	//p_Settings = ofxCYASettings::getInstance();

	panel.setup("CYA:PeopleVision", 20, 20, 300, 700);
	panel.addPanel("image adjustment", 1, false);
	panel.addPanel("sensing", 1, false);
	panel.addPanel("custom", 1, false);
	
	panel.setWhichPanel("image adjustment");
	panel.setWhichColumn(0);
	
	panel.addSlider("threshold", "THRESHOLD", 40, 0, 255, false);
	panel.addToggle("use smoothing", "USE_SMOOTHING", false);
	panel.addSlider("smooth amount", "SMOOTH_AMOUNT", 0, 0, 15, false);
	panel.addToggle("use highpass", "USE_HIGHPASS", true);
	panel.addSlider("highpass blur", "HIGHPASS_BLUR", 1, 1, 200, true);
	panel.addSlider("highpass noise", "HIGHPASS_NOISE", 1, 1, 30, true);
	panel.addToggle("use amplification", "USE_AMPLIFICATION", false);
	panel.addSlider("amplification amount", "AMPLIFICATION_AMOUNT", 1, 1, 200, true);
	
	panel.setWhichPanel("sensing");
	panel.setWhichColumn(0);
	//TODO optionally disable people
	//panel.addToggle("track people", "TRACK_PEOPLE", false);
	panel.addToggle("learn background", "LEARN_BACKGROUND", false);
	//JG 12/8/09 GUI-REDUX Removing this feature
	//gui.addToggle("smart learn background", &p_Settings->bSmartLearnBackground);
	panel.addSlider("minimum blob (screen %)", "MIN_BLOB", .01f, 0.0001f, 1.0, false);
	panel.addSlider("maximum blob (screen %)", "MAX_BLOB", .5f, 0.0001f, 1.0, false);
	panel.addToggle("use progressive relearn", "RELEARN", false);
	panel.addSlider("background relearn rate", "RELEARN_BACKGROUND", .1f, 0.0f, 1000.0f, false);
	panel.addToggle("find holes", "FIND_HOLES", false);
	panel.addToggle("sense optical flow", "SENSE_OPTICAL_FLOW", true);
	
	vector<string> multi;
	multi.push_back("light on dark");
	multi.push_back("dark on light");
	multi.push_back("absolute difference");
	panel.addMultiToggle("blob type", "BLOB_TYPE", 0, multi);
	panel.addToggle("sense haar features (e.g. faces)", "SENSE_HAAR", false);
	//JG 12/8/09 GUI REDUXTODO Add a default set of HAAR files as a multi
	//vector<string> haaFiles; ...
	panel.addSlider("haar ROI padding", "HAAR_PADDING", 0.0f, 0.0f, 200.0f, false);
	//JG GUI-REDUX: removing this feature
	//gui.addToggle("send haar center as blob center", &p_Settings->bUseHaarAsCenter);
	panel.addSlider("min. checkable haar size (%)", "MIN_HAAR", .1f, 0.0001f, 1.0f, false);
	panel.addSlider("max. checkable haar size (%)", "MAX_HAAR", .5f, 0.0001f, 1.0f, false);
	
	//JG TODO: Optionally change config file through the UI
	//this would be a big help for setting up multiple install sites and having those setting
	//included in repositories
	panel.loadSettings("settings/settings.xml");

}

void ofxCYAGuiManager::addSlider(string name, int* value, int min, int max)
{
	ofxCYAGUICustomParam p;
	string key = "CUSTOM"+params.size();
	
	panel.setWhichPanel("custom");
	panel.setWhichColumn(0);	
	panel.addSlider(name, key, *value, min, max, true);
	
	p.key = key;
	p.type = PARAM_INT;
	p.i = value;
	params.push_back(p);
}

void ofxCYAGuiManager::addSlider(string name, float* value, float min, float max)
{
	ofxCYAGUICustomParam p;
	string key = "CUSTOM"+params.size();
	
	panel.setWhichPanel("custom");
	panel.setWhichColumn(0);
	panel.addSlider(name, key, *value, min, max, false);
	
	p.key = key;
	p.type = PARAM_FLOAT;
	p.f = value;
	params.push_back(p);
}

void ofxCYAGuiManager::addToggle(string name, bool* value)
{
	ofxCYAGUICustomParam p;	
	string key = "CUSTOM"+params.size();
	
	panel.setWhichPanel("custom");
	panel.setWhichColumn(0);
	panel.addToggle(name, key, *value);
	
	p.key = key;
	p.type = PARAM_BOOL;
	p.b = value;
	params.push_back(p);
}

void ofxCYAGuiManager::update(ofEventArgs &e)
{
	if(!enableGui){
		//if the gui is not shown no need to propagate values
		return;
	}
	
	ofxCYASettings *p_Settings;
	p_Settings = ofxCYASettings::getInstance();
	
	panel.update();

	p_Settings->threshold = panel.getValueF("THRESHOLD");
	p_Settings->bSmooth = panel.getValueB("USE_SMOOTHING");
	p_Settings->smooth = panel.getValueF("SMOOTH_AMOUNT");
	p_Settings->bHighpass = panel.getValueI("USE_HIGHPASS");
	p_Settings->highpassBlur =  panel.getValueI("HIGHPASS_BLUR");
	p_Settings->highpassNoise = panel.getValueI("HIGHPASS_NOISE");
	p_Settings->bAmplify = panel.getValueB("USE_AMPLIFICATION");
	p_Settings->highpassAmp = panel.getValueI("AMPLIFICATION_AMOUNT");
	
	//TODO optionally disable people
	//panel.addToggle("track people", "TRACK_PEOPLE", false);
	if(!p_Settings->bLearnBackground){ // a little weird b/c it's a one-shot toggle
		p_Settings->bLearnBackground = panel.getValueB("LEARN_BACKGROUND");
	}
	//panel.setValueB("LEARN_BACKGROUND", p_Settings->bLearnBackground);
	//JG 12/8/09 GUI-REDUX Removing this feature
	//gui.addToggle("smart learn background", &p_Settings->bSmartLearnBackground);
	p_Settings->minBlob = panel.getValueF("MIN_BLOB");
	p_Settings->maxBlob = panel.getValueF("MAX_BLOB");
	p_Settings->bLearnBackgroundProgressive = panel.getValueB("RELEARN");
	p_Settings->fLearnRate = panel.getValueB("RELEARN_BACKGROUND");
	p_Settings->bFindHoles = panel.getValueB("FIND_HOLES");
	p_Settings->bTrackOpticalFlow = panel.getValueB("SENSE_OPTICAL_FLOW");
	//JG 12/8/09 GUI-REDUX:
	//TODO Add Optical flow MIN/MAX vector filtering - super useful for noisey scenes
	p_Settings->trackType = panel.getValueI("BLOB_TYPE");
	p_Settings->bDetectHaar = panel.getValueB("SENSE_HAAR");

	//JG 12/8/09 GUI REDUXTODO Add a default set of HAAR files as a multi
	//vector<string> haaFiles; ...
	p_Settings->haarArea = panel.getValueF("HAAR_PADDING");
	//JG GUI-REDUX: removing this feature
	//gui.addToggle("send haar center as blob center", &p_Settings->bUseHaarAsCenter);
	p_Settings->minHaarArea = panel.getValueF("MIN_HAAR");
	p_Settings->maxHaarArea = panel.getValueF("MAX_HAAR");
	
	//modify custom parameters
	vector<ofxCYAGUICustomParam>::iterator it;
	for(it = params.begin(); it != params.end(); it++){
		ofxCYAGUICustomParam p = *it;
		switch (p.type) {
			case PARAM_INT:
				*p.i = panel.getValueI(p.key);
				break;
			case PARAM_FLOAT:
				*p.f = panel.getValueF(p.key);
				break;
			case PARAM_BOOL:
				*p.b = panel.getValueB(p.key);
				break;
			default:
				printf("ofxCYAGUIManager: WARNING undefined parameter type encountered\n");
				break;
		}
	}
}

//forward to gui
void ofxCYAGuiManager::mousePressed(ofMouseEventArgs &e)
{
	if(enableGui) panel.mousePressed(e.x, e.y, e.button);
}

void ofxCYAGuiManager::mouseDragged(ofMouseEventArgs &e)
{
	if(enableGui) panel.mouseDragged(e.x, e.y, e.button);
}

void ofxCYAGuiManager::mouseReleased(ofMouseEventArgs &e)
{
	if(enableGui) panel.mouseReleased();
}

void ofxCYAGuiManager::draw(ofEventArgs &e) {
	if(enableGui) panel.draw();
}
//JG TODO ADD EVENT UNREGISTER FO CLEAN UP

