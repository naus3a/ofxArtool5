//
//  ofxARTPattern.cpp
//  artool5example
//
//  Created by nausea on 12/23/15.
//
//

#include "ofxARTPattern.h"

using namespace ofxArtool5;

PatternTracker::PatternTracker(){
    nullArtPointers();
    bPattFound=false;
    selectedId=-1;
    setScaleFactor(1.0);
    setMinDistance(40.0);
    setMaxDistance(10000.0);
    setPatternWidth(80.0);
}

PatternTracker::~PatternTracker(){
    cleanup();
    nullArtPointers();
}

void PatternTracker::nullArtPointers(){
    gARHandle=NULL;
    gAR3DHandle=NULL;
    gCparamLT=NULL;
    gARPattHandle=NULL;
}

void PatternTracker::cleanup(){
    arPattDetach(gARHandle);
    arPattDeleteHandle(gARPattHandle);
    ar3DDeleteHandle(&gAR3DHandle);
    arDeleteHandle(gARHandle);
    arParamLTFree(&gCparamLT);
}

//SETUP-------------------------------

bool PatternTracker::setup(ofVec2f _camSize, ofVec2f _viewportSize, ofPixelFormat pf, string pthCamParam, string pthMarker){
    bPattFound=false;
    selectedId=-1;
    
    if(!setupCamera(pthCamParam, _camSize, _viewportSize)){
        return false;
    }
    
    arUtilTimerReset();
    
    if(!setupMarker(pthMarker)){
        cleanup();
        return false;
    }
    
    return true;
}

bool PatternTracker::setupCamera(string pthCamParam, ofVec2f _camSize, ofVec2f _viewportSize, ofPixelFormat pf){
    ARParam cParam;
    AR_PIXEL_FORMAT pixFormat = ofxArtool5::toAR(pf);
    
    camSize=_camSize;
    viewportSize=_viewportSize;
    
    if(arParamLoad(ofToDataPath(pthCamParam).c_str(), 1, &cParam)){
        ofLogError("ofxArtool5::setupCamera(), error loading param file");
        return false;
    }
    
    if(cParam.xsize!=camSize.x||cParam.ysize!=camSize.y){
        ofLogWarning("ofxArtool5::setupCamera()","camera param needs resizing");
        arParamChangeSize(&cParam, camSize.x, camSize.y, &cParam);
    }
    
    if((gCparamLT = arParamLTCreate(&cParam, AR_PARAM_LT_DEFAULT_OFFSET))==NULL){
        ofLogError("ofxArtool5::setupCamera()","Error: arParamLTCreate");
        return false;
    }
    
    if((gARHandle = arCreateHandle(gCparamLT))==NULL){
        ofLogError("ofxArtool5::setupCamera()","Error: arCreateHandle");
        return false;
    }
    
    if(arSetPixelFormat(gARHandle, pixFormat)<0){
        ofLogError("ofxArtool5::setupCamera()","Error arSetPixelFormat");
        return false;
    }
    
    if(arSetDebugMode(gARHandle, AR_DEBUG_DISABLE)<0){
        ofLogError("ofxArtool5::setupCamera()","Error arSetDebugMode");
        return false;
    }
    
    if((gAR3DHandle = ar3DCreateHandle(&cParam))==NULL){
        ofLogError("ofxArtool5::setupCamera()","Error ar3DCreateHandle");
        return false;
    }
    
    cvColorFrame.allocate(camSize.x, camSize.y);
    
    return true;
}

bool PatternTracker::setupMarker(string pthMarker){
    if((gARPattHandle = arPattCreateHandle())==NULL){
        ofLogError("ofxArtool5::setupMarker()","Error arPattCreateHandle");
        return false;
    }
    
    if((pattId = arPattLoad(gARPattHandle, ofToDataPath(pthMarker).c_str()))<0){
        ofLogError("ofxArtool5::setupMarker()","Error arPattLoad");
        arPattDeleteHandle(gARPattHandle);
        return false;
    }
    
    arPattAttach(gARHandle, gARPattHandle);
    
    return true;
}

//UPDATE------------------------------
/*void PatternTracker::update(ofPixels &pix){
    cvColorFrame.setFromPixels(pix);
    ARUint8 * arImg = toAR(cvColorFrame);
    update(arImg);
}*/

void PatternTracker::update(ARUint8 *arPix){
    int rD = arDetectMarker(gARHandle, arPix);
    if(rD<0){
        ofLogError("ofxArtool5::update(ARUint8 * arPix)","ERROR: arDetectMarker");
    }
    
    int k = getHighConfMarker();
    calcMarkerTransformation(k);
}

//DRAW--------------------------------
void PatternTracker::drawDebug(){
    ofPushStyle();
    if(isFound()){
        ofSetColor(ofColor::green);
        ofDrawBitmapString("marker FOUND", 10, 10);
        
        ARMarkerInfo marker = getSelectedMarker();
        ofVec2f ctr = getMarkerCenter(marker);
        
        ofSetColor(ofColor::yellow);
        ofFill();
        ofDrawCircle(ctr.x,ctr.y,10);
        ofNoFill();
        ofSetLineWidth(3);
        ofBeginShape();
        for(int i=0;i<4;i++){
            ofVec2f vv = getMarker2DPoint(marker, i);
            ofVertex(vv.x,vv.y);
        }
        ofEndShape(true);
        ofSetLineWidth(1);
    }else{
        ofSetColor(ofColor::red);
        ofDrawBitmapString("marker NOT FOUND", 10, 10);
    }
    ofPopStyle();
}

//MARKER------------------------------
int PatternTracker::getHighConfMarker(){
    int mId = -1;
    for(int i=0;i<gARHandle->marker_num;i++){
        if(gARHandle->markerInfo[i].id==pattId){
            if(mId==-1){
                mId=i;
            }else if(gARHandle->markerInfo[i].cf>gARHandle->markerInfo[mId].cf){
                mId=i;
            }
        }
    }
    selectedId = mId;
    return mId;
}

void PatternTracker::calcMarkerTransformation(int &mId){
    if(mId!=-1){
        // Get the transformation between the marker and the real camera into gPatt_trans.
        ARdouble err = arGetTransMatSquare(gAR3DHandle, &(gARHandle->markerInfo[mId]), gPatt_width, gPatt_trans);
        bPattFound=true;
    }else{
        bPattFound=false;
    }
}

//MATRICES---------------------------
void PatternTracker::loadProjectionMatrix(){
    getARProjectionMatrix(p);
    glMatrixMode(GL_PROJECTION);
#ifdef ARDOUBLE_IS_FLOAT
    glLoadMatrixf(p);
#else
    glLoadMatrixd(p);
#endif
    glMatrixMode(GL_MODELVIEW);
    
    glLoadIdentity();
}

void PatternTracker::loadModelMatrix(){
    getARModelMatrix(m);
#ifdef ARDOUBLE_IS_FLOAT
    glLoadMatrixf(m);
#else
    glLoadMatrixd(m);
#endif
}

void PatternTracker::beginAR(){
    ofRectangle r(0,0,viewportSize.x,viewportSize.y);
    beginAR(r);
}

void PatternTracker::beginAR(ofRectangle & rViewport){
    ofPushView();
    ofViewport(rViewport);
    loadProjectionMatrix();
    loadModelMatrix();
}

void PatternTracker::endAR(){
    ofPopView();
}

//MISC-------------------------------
int PatternTracker::getThreshold(){
    int thr=0;
    if(getThreshMode()==AR_LABELING_THRESH_MODE_MANUAL){
        arGetLabelingThresh(gARHandle, &thr);
    }
    return thr;
}

void PatternTracker::setThreshold(int thr){
    if(getThreshMode()==AR_LABELING_THRESH_MODE_MANUAL){
        thr = ofClamp(thr, 0, 255);
        arSetLabelingThresh(gARHandle, thr);
    }
}

void PatternTracker::setImageProcMode(int md){
    if(md==AR_IMAGE_PROC_FRAME_IMAGE||md==AR_IMAGE_PROC_FIELD_IMAGE){
        arSetImageProcMode(gARHandle, md);
    }
}