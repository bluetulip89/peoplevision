
#ifndef _TEST_APP
#define _TEST_APP

//#define _USE_LIVE_VIDEO

#include "ofMain.h"
#include "ofxCYA.h"

#ifdef _USE_LIVE_VIDEO

//	look in ofxVideoGrabber to find out about more formats
//	comment this out if you just want to use the normal quicktime stuff

//	NOTE: WE MAY NEED MORE STUFF TO MAKE THIS WIN COMPATIBLE
//	ofxVideoGrabber has some parts that will work, but if you want to
//	use the normal ofVideoGrabber with Win, we'll need to incorporate
//	utils from CCV for PS3 eye, etc

//#define USING_OFX_VIDEOGRABBER
//#define BLOB_TRACKER_VIDEO_FORMAT VID_FORMAT_GREYSCALE
//#define BLOB_TRACKER_COLOR_FORMAT VID_FORMAT_GREYSCALE

	#ifdef USING_OFX_VIDEOGRABBER
		#include "ofxVideoGrabber.h"

		#ifndef BLOB_TRACKER_VIDEO_FORMAT
		#define BLOB_TRACKER_VIDEO_FORMAT VID_FORMAT_RGB 
		#endif

		#ifndef BLOB_TRACKER_COLOR_FORMAT
		#define BLOB_TRACKER_COLOR_FORMAT VID_FORMAT_RGB
		#endif
	#endif

#endif

class testApp : public ofBaseApp, public ofxPersonListener{
	public:

		void setup();
		void update();
		void draw();
		
		void keyPressed  (int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
			
		void personEntered( int id );
		void personMoved( int id );
		void personWillLeave(int id );
	
		#ifdef _USE_LIVE_VIDEO
			#ifdef USING_OFX_VIDEOGRABBER
				ofxVideoGrabber vidGrabber;
			#else
				ofVideoGrabber vidGrabber;
			#endif
		#else
			ofVideoPlayer 		vidPlayer;
		#endif
	
	
		ofxCYAPeopleTracker peopleTracker;
	#ifdef USING_OFX_VIDEOGRABBER && #if BLOB_TRACKER_COLOR_FORMAT == VID_FORMAT_GREYSCALE
		
	// if you're capturing greyscale images from ofxVideoGrabber
	// you need to only look at greyscale images

		ofxCvGrayscaleImage		colorImg;	
	#else
		ofxCvColorImage		colorImg;
	#endif	
		int camWidth, camHeight;
};

#endif
	
