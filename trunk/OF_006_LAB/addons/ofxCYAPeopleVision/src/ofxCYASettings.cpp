/*
 *  ofxCYASettings.cpp
 *  CYAPeopleVision
 *
 *  Created by Joshua Walton on 12/7/09.
 *  Copyright 2009 Lab at Rockwell Group. All rights reserved.
 *
 */

#include "ofxCYASettings.h"

bool ofxCYASettings::instanceFlag = false;
ofxCYASettings* ofxCYASettings::single = NULL;

ofxCYASettings* ofxCYASettings::getInstance()
{
    if(! instanceFlag)
    {
        single = new ofxCYASettings();
        instanceFlag = true;
		single->bLearnBackground = false;
		single->bTrackOpticalFlow = true;
		
		single->bLearnBackgroundProgressive = true;
		single->bTrackDark = false;
		single->bLearnBackground = true;
		
		single->threshold = 80;
		single->highpassBlur = 0;
		single->highpassNoise = 0;
		single->highpassAmp = 0;
		single->smooth = 0; 
		
		single->bAmplify = single->bSmooth = single->bHighpass = false;
		single->bFindHoles = true;
		
		single->minBlob = 30;
		single->maxBlob = 640*480;
		
		single->bLearnBackground = true;	
		single->bCentroidDampen = true;
		
        return single;
    }
    else
    {
        return single;
    }
}

void ofxCYASettings::method()
{
    cout << "Method of the singleton class" << endl;
}


