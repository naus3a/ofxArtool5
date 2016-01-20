//
//  ofxArtool5.cpp
//  artool5example
//
//  Created by nausea on 12/9/15.
//
//

#include "ofxArtool5.h"

ofxArtool5::ofxArtool5(){
    curArtMode = NULL;
    nullArtPointers();
    bPattFound=false;
    selectedId=-1;
    setScaleFactor(1.0);
    setMinDistance(40.0);
    setMaxDistance(10000.0);
    gPatt_width = 80.0;
    
    threshNames.resize(6);
    threshNames[0] = "MANUAL";
    threshNames[1] = "AUTO MEDIAN";
    threshNames[2] = "AUTO OTSU";
    threshNames[3] = "AUTO ADAPTIVE";
    threshNames[4] = "AUTO BRACKETING";
    threshNames[5] = "UNKNOWN";
    
    artMode = ART_PATTERN;
    markersNFTCount = 0;
    surfaceSetCount = 0;
}

ofxArtool5::~ofxArtool5(){
    cleanup();
    nullArtPointers();
    delete curArtMode;
}

void ofxArtool5::nullArtPointers(){
    gARHandle=NULL;
    gAR3DHandle=NULL;
    gCparamLT=NULL;
    gARPattHandle=NULL;
    
    kpmHandle=NULL;
    ar2Handle=NULL;
    markersNFT=NULL;
    threadHandle=NULL;
}

void ofxArtool5::cleanup(){
    if(artMode==ART_PATTERN){
        arPattDetach(gARHandle);
        arPattDeleteHandle(gARPattHandle);
        ar3DDeleteHandle(&gAR3DHandle);
        arDeleteHandle(gARHandle);
        arParamLTFree(&gCparamLT);
    }else if(artMode==ART_NFT){
        if(markersNFT)deleteMarkers(&markersNFT, &markersNFTCount);
        unloadNFTData();
        ar2DeleteHandle(&ar2Handle);
        kpmDeleteHandle(&kpmHandle);
        arParamLTFree(&gCparamLT);
    }
}

void ofxArtool5::unloadNFTData(){
    int i,j;
    if(threadHandle){
        trackingInitQuit(&threadHandle);
    }
    j=0;
    for (i = 0; i < surfaceSetCount; i++) {
        ar2FreeSurfaceSet(&surfaceSet[i]);
        j++;
    }
    surfaceSetCount=0;
}

bool ofxArtool5::setupCamera(string pthCamParam, ofVec2f _camSize, ofVec2f _viewportSize, ofPixelFormat pf){
    ARParam cParam;
    AR_PIXEL_FORMAT pixFormat = toAR(pf);
    
    const char * cPathCamParam = ofToDataPath(pthCamParam).c_str();
    
    camSize=_camSize;
    viewportSize=_viewportSize;
    
    if(arParamLoad(cPathCamParam, 1, &cParam)){
        ofLogError("ofxArtool5::setupCamera()", "error loading param file");
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
    
    if(artMode==ART_PATTERN){
        
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
    }else if(artMode==ART_NFT){
        arglCameraFrustumRH(&(gCparamLT->param), getMinDistance(), getMaxDistance(), cameraLens);
    }
    
    cvColorFrame.allocate(camSize.x, camSize.y);
    cvGrayFrame.allocate(camSize.x, camSize.y);
    
    return true;
}

bool ofxArtool5::setupMarker(string pthMarker){
    const char * cPthMarker = ofToDataPath(pthMarker).c_str();
    
    if((gARPattHandle = arPattCreateHandle())==NULL){
        ofLogError("ofxArtool5::setupMarker()","Error arPattCreateHandle");
        return false;
    }
    
    if((pattId = arPattLoad(gARPattHandle, cPthMarker))<0){
        ofLogError("ofxArtool5::setupMarker()","Error arPattLoad");
        arPattDeleteHandle(gARPattHandle);
        return false;
    }
    
    arPattAttach(gARHandle, gARPattHandle);
    
    return true;
}

bool ofxArtool5::initNFT(ARParamLT * cparamLT, AR_PIXEL_FORMAT pixFormat){
    kpmHandle = kpmCreateHandle(cparamLT, pixFormat);
    if (!kpmHandle) {
        ofLogError("ofxArtool5::initNFT","kpmCreateHandle");
        return false;
    }
    
    if((ar2Handle = ar2CreateHandle(cparamLT, pixFormat, AR2_TRACKING_DEFAULT_THREAD_NUM))==NULL){
        ofLogError("ofxArtool5::initNFT","ar2CreateHandle");
        kpmDeleteHandle(&kpmHandle);
        return false;
    }
    
    if(threadGetCPU()<=1){
        ofLogWarning("ofxArtool5::initNFT","Using NFT settings for a single CPU");
        ar2SetTrackingThresh(ar2Handle, 5.0);
        ar2SetSimThresh(ar2Handle, 0.50);
        ar2SetSearchFeatureNum(ar2Handle, 16);
        ar2SetSearchSize(ar2Handle, 6);
        ar2SetTemplateSize1(ar2Handle, 6);
        ar2SetTemplateSize2(ar2Handle, 6);
    }else{
        cout<<"ofxArtool5::initNFT: Using NFT settings for multi CPU"<<endl;
        ar2SetTrackingThresh(ar2Handle, 5.0);
        ar2SetSimThresh(ar2Handle, 0.50);
        ar2SetSearchFeatureNum(ar2Handle, 16);
        ar2SetSearchSize(ar2Handle, 12);
        ar2SetTemplateSize1(ar2Handle, 6);
        ar2SetTemplateSize2(ar2Handle, 6);
    }
    
    return true;
}

bool ofxArtool5::setup(ofVec2f _camSize, ofVec2f _viewportSize, ARToolKitMode _mode){
    artMode = _mode;
    setup(_camSize, _viewportSize);
}

bool ofxArtool5::setup(ofVec2f _camSize, ofVec2f _viewportSize){
    switch (artMode) {
        case ART_PATTERN:
            return setupPattern(_camSize, _viewportSize);
            break;
        case ART_NFT:
            return setupNFT(_camSize, _viewportSize);
            break;
            
        default:
            return false;
            break;
    }
}

bool ofxArtool5::setupPattern(ofVec2f _camSize, ofVec2f _viewportSize, ofPixelFormat pf, string pthCamParam, string pthMarker){
    
    artMode=ART_PATTERN;
    curArtMode=(ofxArtoolMode*)(new ofxArtoolModePatt());
    curArtMode->gCparamLT=gCparamLT;
    
    if(!setupCamera(pthCamParam, _camSize, _viewportSize)){
        return false;
    }
    
    return true;
    
    artMode=ART_PATTERN;
    curArtMode = (ofxArtoolMode * )(new ofxArtoolModePatt());
    curArtMode->gCparamLT = gCparamLT;
    
    bPattFound = false;
    selectedId = -1;
    
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

bool ofxArtool5::setupNFT(ofVec2f _camSize, ofVec2f _viewportSize, ofPixelFormat pf, string pthCamParam, string pthMarkerData){
    artMode=ART_NFT;
    bPattFound = false;
    
    if(!setupCamera(pthCamParam, _camSize, _viewportSize)){
        return false;
    }
    
    if(!initNFT(gCparamLT, toAR(pf))){
        return false;
    }
    
    arUtilTimerReset();
    
    const char * cMarkerData = ofToDataPath(pthMarkerData).c_str();
    newMarkers(cMarkerData, &markersNFT, &markersNFTCount);
    if(!markersNFTCount){
        ofLogError("ofxArtool5::setupNFT","error loading markers from file");
        cleanup();
        return false;
    }
    
    return true;
}

void ofxArtool5::update(ofPixels & pix){
    cvColorFrame.setFromPixels(pix);
    //cvGrayFrame = cvColorFrame;
    
    ARUint8 * arImg = toAR(cvColorFrame);
    update(arImg);
}

void ofxArtool5::update(ARUint8 * arPix){
    //ofNotifyEvent(evARUpdate);
}

void ofxArtool5::updatePattern(ARUint8 *arPix){
    int rD = arDetectMarker(gARHandle, arPix);
    if(rD<0){
        ofLogError("ofxArtool5::update(ARUint8 * arPix)","ERROR: arDetectMarker");
    }
    
    int k = getHighConfMarker();
    calcMarkerTransformation(k);
}

void ofxArtool5::drawDebug(){
    ofPushStyle();
    if(isFound()){
        ofSetColor(ofColor::green);
        ofDrawBitmapString("marker FOUND", 20, 20);
        ofSetColor(ofColor::yellow);
        ofFill();
        ARMarkerInfo marker = getSelectedMarker();
        ofVec2f ctr = getMarkerCenter(marker);
        ofDrawCircle(ctr.x,ctr.y,10);
        ofSetLineWidth(3);
        ofNoFill();
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

void ofxArtool5::loadProjectionMatrix(){
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

void ofxArtool5::loadModelMatrix(){
    getARModelMatrix(m);
#ifdef ARDOUBLE_IS_FLOAT
    glLoadMatrixf(m);
#else
    glLoadMatrixd(m);
#endif
}

void ofxArtool5::beginAR(){
    ofPushView();
    loadProjectionMatrix();
    loadModelMatrix();
}

void ofxArtool5::endAR(){
    ofPopView();
}

int ofxArtool5::getHighConfMarker(){
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

void ofxArtool5::calcMarkerTransformation(int & mId){
    
    if(mId!=-1){
        // Get the transformation between the marker and the real camera into gPatt_trans.
        ARdouble err = arGetTransMatSquare(gAR3DHandle, &(gARHandle->markerInfo[mId]), gPatt_width, gPatt_trans);
        bPattFound=true;
    }else{
        bPattFound=false;
    }
}

vector<ofVec2f> ofxArtool5::getMarker2DCorners(ARMarkerInfo &marker){
    vector<ofVec2f> crn;
    crn.clear();
    for(int i=0;i<4;i++){
        crn.push_back(getMarker2DPoint(marker, i));
    }
    return crn;
}

vector<ofVec2f> ofxArtool5::getMarker2DOrderedCorners(ARMarkerInfo &marker){
    vector<ofVec2f> crn;
    crn.clear();
    int index = 4 - marker.dir;
    for(int i=0;i<4;i++){
        crn.push_back(getMarker2DPoint(marker, index));
        index++;
        if(index>=4)index=0;
    }
    return crn;
}

AR_PIXEL_FORMAT ofxArtool5::toAR(ofPixelFormat & ofPf){
    AR_PIXEL_FORMAT npf = AR_PIXEL_FORMAT_INVALID;
    switch(ofPf){
        case OF_PIXELS_GRAY:
            npf = AR_PIXEL_FORMAT_MONO;
            break;
        case OF_PIXELS_RGB:
            npf = AR_PIXEL_FORMAT_RGB;
            break;
        case OF_PIXELS_BGR:
            npf = AR_PIXEL_FORMAT_BGR;
            break;
        case OF_PIXELS_RGBA:
            npf = AR_PIXEL_FORMAT_RGBA;
            break;
        case OF_PIXELS_BGRA:
            npf = AR_PIXEL_FORMAT_BGRA;
            break;
        case OF_PIXELS_RGB565:
            npf = AR_PIXEL_FORMAT_RGB_565;
            break;
        case OF_PIXELS_NV21:
            npf = AR_PIXEL_FORMAT_NV21;
            break;
        default:
            break;
    }
    return npf;
}

string ofxArtool5::getThresholdModeName(AR_LABELING_THRESH_MODE tm){
    string n;
    switch(tm){
        case AR_LABELING_THRESH_MODE_MANUAL:
            n = threshNames[0];
            break;
        case AR_LABELING_THRESH_MODE_AUTO_MEDIAN:
            n = threshNames[1];
            break;
        case AR_LABELING_THRESH_MODE_AUTO_OTSU:
            n = threshNames[2];
            break;
        case AR_LABELING_THRESH_MODE_AUTO_ADAPTIVE:
            n = threshNames[3];
            break;
        case AR_LABELING_THRESH_MODE_AUTO_BRACKETING:
            n = threshNames[4];
            break;
        default:
            n = threshNames[5];
            break;
    }
    return n;
}

int ofxArtool5::getThreshold(){
    int thr=0;
    if(getThreshMode()==AR_LABELING_THRESH_MODE_MANUAL){
        arGetLabelingThresh(gARHandle, &thr);
    }
    return thr;
}

void ofxArtool5::setThreshold(int thr){
    if(getThreshMode()==AR_LABELING_THRESH_MODE_MANUAL){
        thr = ofClamp(thr, 0, 255);
        arSetLabelingThresh(gARHandle, thr);
    }
}

string ofxArtool5::getImageProcModeName(int md){
    string n;
    switch(md){
        case AR_IMAGE_PROC_FRAME_IMAGE:
            n="AR_IMAGE_PROC_FRAME_IMAGE";
            break;
        case AR_IMAGE_PROC_FIELD_IMAGE:
            n="AR_IMAGE_PROC_FIELD_IMAGE";
            break;
        
        default:
            n="UNKNOWN";
            break;
    }
    return n;
}

void ofxArtool5::setImageProcMode(int md){
    if(md==AR_IMAGE_PROC_FRAME_IMAGE||md==AR_IMAGE_PROC_FIELD_IMAGE){
        arSetImageProcMode(gARHandle, md);
    }
}
