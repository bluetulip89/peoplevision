#ifndef _TEST_APP
#define _TEST_APP

#define _USE_LIVE_VIDEO

/******************************************************************
	TRACKING + FILTER SCRIPT FROM NUI CCV
******************************************************************/

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCvHaarTracker.h"
#include "ofxSimpleGuiToo.h"
#include "CPUImageFilter.h"

class testApp : public ofBaseApp{
	public:

		void setup();
		void update();
		void draw();
		
		void keyPressed  (int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
			
		#ifdef _USE_LIVE_VIDEO
			ofVideoGrabber 		vidGrabber;
		#else
			ofVideoPlayer 		vidPlayer;
		#endif
	
		ofxCvColorImage		colorImg;
		ofxCvColorImage		colorSmallImage;
	
		ofxCvGrayscaleImage	grayImage;
		ofxCvGrayscaleImage	grayLastImage;
	
		ofxCvGrayscaleImage grayBg;
		ofxCvGrayscaleImage subtractBg;
		ofxCvGrayscaleImage graySmallImage;
		CPUImageFilter		grayDiff;
		ofxCvShortImage		floatBgImg;
	
		ofxCvContourFinder 	contourFinder;
	
		int smooth;
		int highpassBlur;
		int highpassNoise;
		int	highpassAmp;
		int threshold;	
		int minBlob;
		int maxBlob;
	
		float haarArea;
		float minHaarArea;
		float maxHaarArea;
	
		float fLearnRate;
		float camWidth, camHeight;
		
		bool bFindHoles;
		bool bLearnBackground;
		bool bSmartLearnBackground;
		bool bLearnBackgroundProgressive;
		bool bTrackDark;
		bool bHighpass;
		bool bAmplify;
		bool bSmooth;
		bool bDetectHaar;
		bool bTrackOpticalFlow;
	
		ofxCvHaarFinder			haarFinder;
		ofxCvHaarTracker		haarTracker;
	
		ofxCvOpticalFlowLK	opticalFlow;
	
		ofxSimpleGuiToo	gui;
};

#endif
	
