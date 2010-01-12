
#include "ofxCYAPeopleTracker.h"
#include "CPUImageFilter.h"

//scales down tracking images for improved performance
#define TRACKING_SCALE_FACTOR .5

#pragma mark Setup
void ofxCYAPeopleTracker::setup(int w, int h)
{
	width  = w;
	height = h;
	
	grayImage.allocate(width, height);
	grayBg.allocate(width, height);
	grayDiff.allocate(width, height);
	floatBgImg.allocate(width, height);
	graySmallImage.allocate( width*TRACKING_SCALE_FACTOR, height*TRACKING_SCALE_FACTOR );
	
	grayLastImage.allocate( width*TRACKING_SCALE_FACTOR, height*TRACKING_SCALE_FACTOR );
	grayBabyImage.allocate( width*TRACKING_SCALE_FACTOR, height*TRACKING_SCALE_FACTOR );
	
	//set up optical flow
	opticalFlow.allocate( width*TRACKING_SCALE_FACTOR, height*TRACKING_SCALE_FACTOR );
	opticalFlow.setCalcStep(5,5);
	
	//set tracker
	setHaarXMLFile("HS.xml");
		
	p_Settings = ofxCYASettings::getInstance();
	
	//gui.loadFromXML();	
	//gui.setDraw(true);	
	
    persistentTracker.setListener( this );
}

void ofxCYAPeopleTracker::setHaarXMLFile(string haarFile)
{
	haarFinder.setup(haarFile);
	haarTracker.setup(&haarFinder);

}
void ofxCYAPeopleTracker::setupTuio(string ip, int port)
{
	bTuioEnabled = true;
	tuioClient.setup(ip, port);
}

void ofxCYAPeopleTracker::setListener(ofxPersonListener* listener)
{
	eventListener = listener;
}

#pragma mark Track People
void ofxCYAPeopleTracker::update(ofxCvColorImage image)
{
	grayImage = image;
	trackPeople();
}

void ofxCYAPeopleTracker::update(ofxCvGrayscaleImage image)
{
	grayImage = image;
	trackPeople();
}

/**
 * Core Method
 * Run every frame to update
 * the system to the current location
 * of people
 */
void ofxCYAPeopleTracker::trackPeople()
{
	graySmallImage.scaleIntoMe(grayImage);
	grayBabyImage.scaleIntoMe(grayImage);
	
	//-------------------
	//BACKGROUND
	//-------------------
	
	//force learn background if there are > 5 blobs (off by default)
	//JG Disabling this feature for now, 
	//I think it's a great ideas but it needs to be better described and "5" needs to be customizable
//	if (p_Settings->bSmartLearnBackground == true && contourFinder.nBlobs > 5){
//		p_Settings->bLearnBackground = true;
//	}
	
	//learn background (either in reset or additive)
	if (p_Settings->bLearnBackground){
		cout << "Learning Background" << endl;
		grayBg = grayImage;
	}
	else if (p_Settings->bLearnBackgroundProgressive){
		floatBgImg.addWeighted( grayImage, p_Settings->fLearnRate * .0001);
		cvConvertScale( floatBgImg.getCvImage(), grayBg.getCvImage(), 255.0f/65535.0f, 0 );       
		grayBg.flagImageChanged();			
	}
	
	//printf("track type %d from (%d,%d,%d)\n", p_Settings->trackType, TRACK_ABSOLUTE, TRACK_DARK, TRACK_LIGHT);
	if(p_Settings->trackType == TRACK_ABSOLUTE){
		grayDiff.absDiff(grayBg, grayImage);
	}
	else{
		grayDiff = grayImage;
		if(p_Settings->trackType == TRACK_LIGHT){
			//grayDiff = grayBg - grayImage;
			cvSub(grayBg.getCvImage(), grayDiff.getCvImage(), grayDiff.getCvImage());
		}
		else if(p_Settings->trackType == TRACK_LIGHT){ 
			cvSub(grayDiff.getCvImage(), grayBg.getCvImage(), grayDiff.getCvImage());
			//grayDiff = grayImage - grayBg;
		}
		grayDiff.flagImageChanged();
	}
	
	//-----------------------
	// IMAGE TREATMENT
	//-----------------------
	if(p_Settings->bSmooth){
		grayDiff.blur((p_Settings->smooth * 2) + 1); //needs to be an odd number
	}
	
	//highpass filter (see cpuimagefilter class)	
	if(p_Settings->bHighpass){
		grayDiff.highpass(p_Settings->highpassBlur, p_Settings->highpassNoise);
	}
	
	//amplify (see cpuimagefilter class)
	if(p_Settings->bAmplify){
		grayDiff.amplify(grayDiff, p_Settings->highpassAmp);
	}
	//threshold	
	grayDiff.threshold(p_Settings->threshold);
	
	//-----------------------
	// TRACKING
	//-----------------------	
	if (p_Settings->bTrackOpticalFlow){
		opticalFlow.calc(grayLastImage, graySmallImage, 11);
	}
	
	contourFinder.findContours(grayDiff, p_Settings->minBlob*width*height, p_Settings->maxBlob*width*height, 50, p_Settings->bFindHoles);
	persistentTracker.trackBlobs(contourFinder.blobs);
	
	scene.averageMotion = opticalFlow.flowInRegion(0,0,width,height);
	scene.percentCovered = 0; 
	
	for(int i = 0; i < persistentTracker.blobs.size(); i++){
		ofxCvTrackedBlob blob = persistentTracker.blobs[i];
		ofxCYAPerson* p = getTrackedPerson(blob.id);
		//somehow we are not tracking this person, safeguard (shouldn't happen)
		if(NULL == p){
			printf("ofxPerson::warning. encountered persistent blob without a person behind them\n");
			continue;
		}
		
		scene.percentCovered += blob.area;
		
		//update this person with new blob info
		p->update(blob, p_Settings->bCentroidDampen);

		//sum optical flow for the person
		if(p_Settings->bTrackOpticalFlow){
			p->opticalFlowVectorAccumulation = opticalFlow.flowInRegion(p->boundingRect);
		}
		
		//detect haar patterns (faces, eyes, etc)
		if (p_Settings->bDetectHaar){
			int blobId, oldId;
			bool bHaarItemSet = false;
			if (p->area > p_Settings->minHaarArea && p->area < p_Settings->maxHaarArea){
				
				//find the region of interest, expanded by haarArea.
				//bound by the frame edge
				ofRectangle roi;
				roi.x		= fmax( (p->boundingRect.x - p_Settings->haarArea) * TRACKING_SCALE_FACTOR, 0.0f );
				roi.y		= fmax( (p->boundingRect.y - p_Settings->haarArea) * TRACKING_SCALE_FACTOR, 0.0f );
				roi.width	= fmin( (p->boundingRect.width  + p_Settings->haarArea*2) * TRACKING_SCALE_FACTOR, grayBabyImage.width - roi.x );
				roi.height	= fmin( (p->boundingRect.height + p_Settings->haarArea*2) * TRACKING_SCALE_FACTOR, grayBabyImage.width - roi.y );	
				
				haarTracker.findHaarObjects( grayBabyImage, roi );
				float x, y, w, h;
				while( haarTracker.hasNextHaarItem() ) {
					blobId = haarTracker.getHaarItemID();
					haarTracker.getHaarItemPropertiesEased( &x, &y, &w, &h );
					//strange bug where features in external ROI's are being detected. 
					//tried to call clearHaarItems() between blobs but this causes no detections at all
					//work around for now is ensure the rectangle is contained in the ROI
					if(roi.x < x && roi.y < y && roi.width > w && roi.height > h){
						//mult by two since haar finder is looking at a 320x240 sample
						x /= TRACKING_SCALE_FACTOR;
						y /= TRACKING_SCALE_FACTOR;
						w /= TRACKING_SCALE_FACTOR;
						h /= TRACKING_SCALE_FACTOR;
						p->setHaarRect(ofRectangle(x, y, w, h));
						bHaarItemSet = true;
						break;	//only allow first haar item
					}
				}
			}
			//flag that we missed a frame
			if(!bHaarItemSet){
				p->noHaarThisFrame();
			}
		}
		
		if(eventListener != NULL){
			if( p->velocity.x != 0 || p->velocity.y != 0){
				eventListener->personMoved(p, &scene);
			}
			eventListener->personUpdated(p, &scene);
		}
	}
	
	//normalize it
	scene.percentCovered /= width*height;
	
	if(bTuioEnabled){
		for (int i = 0; i < trackedPeople.size(); i++){
			ofxCYAPerson* p = trackedPeople[i];
			if(p_Settings->bUseHaarAsCenter && p->hasHaarRect()){
				ofPoint tuioCursor = p->getHaarCentroidNormalized(width, height);
				tuioClient.cursorDragged( tuioCursor.x, tuioCursor.y, p->oid);
			}
			else{
				ofPoint tuioCursor = p->getCentroidNormalized(width, height);
				tuioClient.cursorDragged( tuioCursor.x, tuioCursor.y, p->oid);
			}
		}
		
		tuioClient.update();		
	}
	
	//store the old image
	grayLastImage = graySmallImage;	
}

#pragma mark Person Management
void ofxCYAPeopleTracker::blobOn( int x, int y, int id, int order )
{
	ofxCvTrackedBlob blob = persistentTracker.getById( id );
	ofxCYAPerson* newPerson = new ofxCYAPerson(id, order, blob);
	trackedPeople.push_back( newPerson );
	if(eventListener != NULL){
		eventListener->personEntered(newPerson, &scene);
	}
	if(bTuioEnabled){
		tuioClient.cursorPressed(1.0*x/width, 1.0*y/height, order);
	}
}

void ofxCYAPeopleTracker::blobMoved( int x, int y, int id, int order ){/*not used*/}

void ofxCYAPeopleTracker::blobOff( int x, int y, int id, int order )
{
	ofxCYAPerson* p = getTrackedPerson(id);
	//ensure we are tracking
	if(NULL == p){
		printf("ofxPerson::warning. encountered persistent blob without a person behind them\n");		
		return;
	}
	
	//alert the delegate
	if(eventListener != NULL){
		eventListener->personWillLeave(p, &scene);
	}
	if (bTuioEnabled) {
		ofPoint cursor = p->getCentroidNormalized(width, height);
		tuioClient.cursorReleased(cursor.x, cursor.y, order);	
	}
	
	//delete the object and remove it from the vector
	std::vector<ofxCYAPerson*>::iterator it;
	for(it = trackedPeople.begin(); it != trackedPeople.end(); it++){
		if((*it)->pid == p->pid){
			trackedPeople.erase(it);
			delete p;
			break;
		}
	}
}

//bool ofxCYAPeopleTracker::isTrackingPerson( int pid )
//{
//    for( int i = 0; i < trackedPeople.size(); i++ ) {
//        if( trackedPeople[i]->pid == pid ) {
//            return true;
//        }
//    }
//	return false;
//}

ofxCYAPerson* ofxCYAPeopleTracker::getTrackedPerson( int pid )
{
    for( int i = 0; i < trackedPeople.size(); i++ ) {
        if( trackedPeople[i]->pid == pid ) {
            return trackedPeople[i];
        }
    }
	return NULL;
}

#pragma mark Draw
void ofxCYAPeopleTracker::draw()
{
	draw(0,0);
}

void ofxCYAPeopleTracker::draw(int x, int y)
{
	draw(x,y,drawMode);
}

void ofxCYAPeopleTracker::draw(int x, int y, int mode)
{

//		mode = DRAW_MODE_NORMAL;
//		switch (mode) {
//			case DRAW_MODE_GUI:
////				gui.draw();
		ofPushMatrix();{
			ofTranslate(x, y, 0);
			//translate to make room for the gui
			ofTranslate(320, 0, 0);
			// draw the incoming, the grayscale, the bg and the thresholded difference
			ofSetColor(0xffffff);
			grayImage.draw(20,20,320,240);
			grayDiff.draw(360,20,320,240);
			grayBg.draw(20,280,320,240);
								
			//individually draw blobs and report findings
			ofPushMatrix();{
				ofTranslate(360,280);
				
				ofFill();
				ofSetColor(0x333333);
				ofRect(0,0,320,240);
				ofSetColor(0xffffff);
				
				ofNoFill();
				if (p_Settings->bTrackOpticalFlow){
					ofSetColor(0x888888);
					opticalFlow.draw(width*TRACKING_SCALE_FACTOR, height*TRACKING_SCALE_FACTOR);
				}					
				
				ofScale(TRACKING_SCALE_FACTOR, TRACKING_SCALE_FACTOR);
				contourFinder.draw();
			
				for (int i=0; i < trackedPeople.size(); i++){
					
					//draw blobs				
					//if haarfinder is looking at these blobs, draw the area it's looking at
					ofxCYAPerson* p = trackedPeople[i];
					
					if(p_Settings->bTrackOpticalFlow){
						//purple optical flow arrow
						ofSetColor(0xff00ff);
						ofLine(p->centroid.x, p->centroid.y, p->centroid.x + p->opticalFlowVectorAccumulation.x, p->centroid.y + p->opticalFlowVectorAccumulation.y);
					}
					
					ofSetColor(0xffffff);							
					if(p_Settings->bDetectHaar){
						//draw haar search area expanded 
						ofRect(p->boundingRect.x - p_Settings->haarArea, 
							   p->boundingRect.y - p_Settings->haarArea, 
							   p->boundingRect.width  + p_Settings->haarArea*2, 
							   p->boundingRect.height + p_Settings->haarArea*2);
					}
					
					if(p->hasHaarRect()){
						//draw the haar rect in green
						ofSetColor(0x00ff00);
						ofRect(p->getHaarRect().x, p->getHaarRect().y, p->getHaarRect().width, p->getHaarRect().height);
						//haar-detected people get a yellow square
						ofSetColor(0xffff00);
					}
					else {
						//no haar gets a blue square
						ofSetColor(0x0000ff);
					}
					
					//draw person
					ofRect(p->boundingRect.x, p->boundingRect.y, p->boundingRect.width, p->boundingRect.height);
					
					//draw centroid
					ofSetColor(0xff0000);
					ofCircle(p->centroid.x, p->centroid.y, 3);
												
					//draw id
					ofSetColor(0xffffff);
					char idstr[1024];
					sprintf(idstr, "pid: %d\noid: %d\nage: %d", p->pid, p->oid, p->age );
					ofDrawBitmapString(idstr, p->centroid.x+8, p->centroid.y);													
				}
			}ofPopMatrix();	//release video feedback drawing
			
//			break;
//			case DRAW_MODE_CAMERA_CALIBRATE:
//				//TODO implement calibration mode
//				//vidGrabber.draw(ofGetWidth()- camWidth,0);			
//				break;
//			case DRAW_MODE_FULLSCREEN:
//				ofSetColor(0xffffff);
//				grayImage.draw(0,0,ofGetWidth(),ofGetHeight());
//		
//				break;
//			default:
//				printf("undefined draw mode: %d\n", mode);
//				break;
		}ofPopMatrix();
}

#pragma mark gui extension
void ofxCYAPeopleTracker::addSlider(string name, int* value, int min, int max)
{
	//forward to the gui manager
	gui.addSlider(name, value, min, max);
}

void ofxCYAPeopleTracker::addSlider(string name, float* value, float min, float max)
{
	gui.addSlider(name, value, min, max);	
}

void ofxCYAPeopleTracker::addToggle(string name, bool* value)
{
	gui.addToggle(name, value);	
}

#pragma mark accessors

/**
 * simple public getter for external classes
 */
ofxCYAPerson* ofxCYAPeopleTracker::personAtIndex(int i)
{
	return trackedPeople[i];
}

int ofxCYAPeopleTracker::totalPeople()
{
	return trackedPeople.size();
}

void ofxCYAPeopleTracker::enableHaarFeatures(bool doHaar)
{
	p_Settings->bDetectHaar = doHaar;
}

void ofxCYAPeopleTracker::enableOpticalFlow(bool doOpticalFlow)
{
	p_Settings->bTrackOpticalFlow = doOpticalFlow;
}

#pragma mark background management
void ofxCYAPeopleTracker::relearnBackground()
{
	p_Settings->bLearnBackground = true;
}

void ofxCYAPeopleTracker::enableBackgroundRelearnSmart(bool doSmartLearn)//auto-relearns if there are too many blobs in the scene
{
	p_Settings->bSmartLearnBackground = doSmartLearn;
}

void ofxCYAPeopleTracker::enableBackgroundReleaernProgressive(bool doProgressive) //relearns over time using progessive frame averagering
{
	p_Settings->bLearnBackgroundProgressive = doProgressive;
}

void ofxCYAPeopleTracker::setRelearnRate(float relearnRate)
{
	p_Settings->fLearnRate = relearnRate;
}


#pragma mark image control
void ofxCYAPeopleTracker::setThreshold(float thresholdAmount)
{
	p_Settings->threshold = thresholdAmount;
}

void ofxCYAPeopleTracker::setMinBlobSize(float minBlobSize)
{
	p_Settings->minBlob = minBlobSize; 
}

void ofxCYAPeopleTracker::setMaxBlobSize(float maxBlobSize)
{
	p_Settings->maxBlob = maxBlobSize;
}

void ofxCYAPeopleTracker::enableSmooth(bool doSmooth)
{
	p_Settings->bSmooth = doSmooth;
}

void ofxCYAPeopleTracker::setSmoothAmount(int smoothAmount)
{
	p_Settings->smooth = smoothAmount;
}

void ofxCYAPeopleTracker::enableHighpass(bool doHighpass)
{
	p_Settings->bHighpass = doHighpass;
}

void ofxCYAPeopleTracker::setHighpassBlurAmount(int highpassBlurAmount)
{
	p_Settings->highpassBlur = highpassBlurAmount;
}

void ofxCYAPeopleTracker::setHighpassNoiseAmount(int highpassNoiseAmount)
{
	p_Settings->highpassNoise = highpassNoiseAmount;
}

void ofxCYAPeopleTracker::enableAmplify(bool doAmp)
{
	p_Settings->bAmplify = doAmp;
}

void ofxCYAPeopleTracker::setAmplifyAmount(int amplifyAmount)
{
	p_Settings->highpassAmp = amplifyAmount;
}

#pragma mark filter controls
//haar
void ofxCYAPeopleTracker::setHaarExpandArea(float haarExpandAmount) //makes the haar rect +area bigger
{
	p_Settings->haarArea = haarExpandAmount;
}

void ofxCYAPeopleTracker::setMinHaarArea(float minArea)
{
	p_Settings->minHaarArea = minArea;
}
void ofxCYAPeopleTracker::setMaxHaarArea(float maxArea)
{
	p_Settings->maxHaarArea = maxArea;
}

void ofxCYAPeopleTracker::useHaarAsCentroid(bool useHaarCenter)
{
	p_Settings->bUseHaarAsCenter = useHaarCenter;
}

//blobs
void ofxCYAPeopleTracker::enableFindHoles(bool findHoles)
{
	p_Settings->bFindHoles = findHoles;
}

void ofxCYAPeopleTracker::trackDarkBlobs()
{
	p_Settings->trackType = TRACK_DARK;
}

void ofxCYAPeopleTracker::trackLightBlobs()
{
	p_Settings->trackType = TRACK_LIGHT;	
}

void ofxCYAPeopleTracker::setDrawMode(int mode)
{
	drawMode = mode;
}

int ofxCYAPeopleTracker::getDrawMode() 
{
	return drawMode;
}
