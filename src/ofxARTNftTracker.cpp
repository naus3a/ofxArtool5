//
//  ofxARTNftTracker.cpp
//
//  Created by nausea on 12/23/15.
//
//

#include "ofxARTNftTracker.h"

using namespace ofxArtool5;


//NftTracker-----------

NftTracker::NftTracker(){
    nullArtPointers();
    markersNFTCount=0;
    surfaceSetCount=0;
    setMinDistance(10.0);
    setMaxDistance(10000.0);
    setScaleFactor(1.0);
    
    detectedPage = int(PAGE_NOT_INITED);
    
    bFound = false;
    selectedId = -1;
}

NftTracker::~NftTracker(){
    cleanup();
    nullArtPointers();
}

void NftTracker::nullArtPointers(){
    gCparamLT=NULL;
    kpmHandle=NULL;
    ar2Handle=NULL;
    markersNFT=NULL;
    threadHandle=NULL;
    gArglSettings=NULL;
}

void NftTracker::cleanup(){
    if(markersNFT)deleteMarkers(&markersNFT, &markersNFTCount);
    
    // NFT cleanup.
    unloadNFTData();
	ARLOGd("Cleaning up ARToolKit NFT handles.\n");
    ar2DeleteHandle(&ar2Handle);
    kpmDeleteHandle(&kpmHandle);
    arParamLTFree(&gCparamLT);
    
    // OpenGL cleanup.
    arglCleanup(gArglSettings);
    gArglSettings = NULL;
}

int NftTracker::unloadNFTData(){
    int i, j;
    
    if (threadHandle) {
        ARLOGi("Stopping NFT2 tracking thread.\n");
        trackingInitQuit(&threadHandle);
    }
    j = 0;
    for (i = 0; i < surfaceSetCount; i++) {
        if (j == 0) ARLOGi("Unloading NFT tracking surfaces.\n");
        ar2FreeSurfaceSet(&surfaceSet[i]); // Also sets surfaceSet[i] to NULL.
        j++;
    }
    if (j > 0) ARLOGi("Unloaded %d NFT tracking surfaces.\n", j);
    surfaceSetCount = 0;
    
    return 0;
}

bool NftTracker::loadNFTData(string pthCustomDat){
    int i;
    KpmRefDataSet *refDataSet;
    
    // If data was already loaded, stop KPM tracking thread and unload previously loaded data.
    if (threadHandle) {
        ARLOGi("Reloading NFT data.\n");
        unloadNFTData();
    } else {
        ARLOGi("Loading NFT data.\n");
    }
    
    refDataSet = NULL;
    
    for (i = 0; i < markersNFTCount; i++) {
        // Load KPM data.
        KpmRefDataSet  *refDataSet2;
        ARLOGi("Reading %s.fset3\n", markersNFT[i].datasetPathname);
        if (kpmLoadRefDataSet(markersNFT[i].datasetPathname, "fset3", &refDataSet2) < 0 ) {
            ARLOGe("Error reading KPM data from %s.fset3\n", markersNFT[i].datasetPathname);
            markersNFT[i].pageNo = -1;
            continue;
        }
        markersNFT[i].pageNo = surfaceSetCount;
        ARLOGi("  Assigned page no. %d.\n", surfaceSetCount);
        if (kpmChangePageNoOfRefDataSet(refDataSet2, KpmChangePageNoAllPages, surfaceSetCount) < 0) {
            ARLOGe("Error: kpmChangePageNoOfRefDataSet\n");
            return false;
        }
        if (kpmMergeRefDataSet(&refDataSet, &refDataSet2) < 0) {
            ARLOGe("Error: kpmMergeRefDataSet\n");
            return false;
        }
        ARLOGi("  Done.\n");
        
        // Load AR2 data.
        ARLOGi("Reading %s.fset\n", markersNFT[i].datasetPathname);
        //if ((surfaceSet[surfaceSetCount] = ar2ReadSurfaceSet(markersNFT[i].datasetPathname, "fset", NULL)) == NULL ) {
        //    ARLOGe("Error reading data from %s.fset\n", markersNFT[i].datasetPathname);
        //}
        
        if((surfaceSet[surfaceSetCount]=loadSurfaceSetData(ofToDataPath(pthCustomDat)))==NULL){
            ARLOGe("Error reading data from custom.dat\n", markersNFT[i].datasetPathname);
        }
        ARLOGi("  Done.\n");
        
        surfaceSet[surfaceSetCount]->surface->markerSet=NULL;
        surfaceSet[surfaceSetCount]->surface->jpegName=NULL;
        
        surfaceSetCount++;
        if (surfaceSetCount == PAGES_MAX) break;
    }
    
    if (kpmSetRefDataSet(kpmHandle, refDataSet) < 0) {
        ARLOGe("Error: kpmSetRefDataSet\n");
        return false;
    }
    
    kpmDeleteRefDataSet(&refDataSet);
    
    // Start the KPM tracking thread.
    threadHandle = trackingInitInit(kpmHandle);
    if (!threadHandle){
        ofLogError("ofxArtool5::loadNFTData","No threadHandle");
        return false;
    }
    ARLOGi("Loading of NFT data complete.\n");
    
    return true;
}

bool NftTracker::initNFT(ARParamLT *cparamLT, AR_PIXEL_FORMAT pixFormat){
    ARLOGd("Initialising NFT.\n");
    
    // KPM init.
    kpmHandle = kpmCreateHandle(cparamLT, pixFormat);
    if (!kpmHandle) {
        ARLOGe("Error: kpmCreateHandle.\n");
        return false;
    }
    
    // AR2 init.
    if( (ar2Handle = ar2CreateHandle(cparamLT, pixFormat, AR2_TRACKING_DEFAULT_THREAD_NUM)) == NULL ) {
        ARLOGe("Error: ar2CreateHandle.\n");
        kpmDeleteHandle(&kpmHandle);
        return false;
    }
    
    if (threadGetCPU() <= 1) {
        ARLOGi("Using NFT tracking settings for a single CPU.\n");
        ar2SetTrackingThresh(ar2Handle, 5.0);
        ar2SetSimThresh(ar2Handle, 0.50);
        ar2SetSearchFeatureNum(ar2Handle, 16);
        ar2SetSearchSize(ar2Handle, 6);
        ar2SetTemplateSize1(ar2Handle, 6);
        ar2SetTemplateSize2(ar2Handle, 6);
    } else {
        ARLOGi("Using NFT tracking settings for more than one CPU.\n");
        ar2SetTrackingThresh(ar2Handle, 5.0);
        ar2SetSimThresh(ar2Handle, 0.50);
        ar2SetSearchFeatureNum(ar2Handle, 16);
        ar2SetSearchSize(ar2Handle, 12);
        ar2SetTemplateSize1(ar2Handle, 6);
        ar2SetTemplateSize2(ar2Handle, 6);
    }
    
    return true;
}

//SETUP------------------------------------
bool NftTracker::setup(ofVec2f _camSize, ofVec2f _viewportSize, ofPixelFormat pf, string pthCamParam, string pthMarkerData, string pthCustomDat){
    
    if(!setupCamera(pthCamParam, _camSize, _viewportSize)){
        ofLogError("ofxArtool5::setup","unable to setup AR camera");
        return false;
    }
    
    //AR init
    
    // Create the OpenGL projection from the calibrated camera parameters.
    arglCameraFrustumRH(&(gCparamLT->param), viewDistanceMin, viewDistanceMax, cameraLens);
    
    AR_PIXEL_FORMAT pixFormat = ofxArtool5::toAR(pf);
    if(!initNFT(gCparamLT, pixFormat)){
        ofLogError("ofxArtool5::setup","unable to init NFT");
        return false;
    }
    
    // Setup ARgsub_lite library for current OpenGL context.
	if ((gArglSettings = arglSetupForCurrentContext(&(gCparamLT->param), pixFormat)) == NULL) {
		ofLogError("ofxArtool5::setup","arglSetupForCurrentContext() returned error");
		cleanup();
		return false;
	}
    
    arUtilTimerReset();
    
    //load markers
    const char * cPathMarkerParam = ofToDataPath(pthMarkerData).c_str();
    newMarkers(cPathMarkerParam, &markersNFT, &markersNFTCount);
    if (!markersNFTCount) {
        ofLogError("ofxArtool5::setup","Error loading markers from config. file "+pthMarkerData);
		cleanup();
		return false;
    }else{
        ARLOGi("Marker count = %d\n", markersNFTCount);
    }
    
    // Marker data has been loaded, so now load NFT data.
    if(!loadNFTData(pthCustomDat)){
        ofLogError("ofxArtool5::setup","Error loading NFT Data");
        cleanup();
        return false;
    }
    
    detectedPage = int(PAGE_NOT_INITED);
    bFound = false;
    selectedId=-1;
    return true;
}

bool NftTracker::setupCamera(string pthCamParam, ofVec2f _camSize, ofVec2f _viewportSize, ofPixelFormat pf){
    ARParam cparam;
    AR_PIXEL_FORMAT pixFormat = ofxArtool5::toAR(pf);
    
    string pth = ofToDataPath(pthCamParam);
    const char * cPathCamParam = pth.c_str();
    
    camSize=_camSize;
    viewportSize=_viewportSize;
    
    if (arParamLoad(cPathCamParam, 1, &cparam) < 0) {
		ARLOGe("setupCamera(): Error loading parameter file %s for camera.\n", cPathCamParam);
        return false;
    }
    
    if (cparam.xsize != camSize.x || cparam.ysize != camSize.y) {
        ARLOGw("*** Camera Parameter resized from %d, %d. ***\n", cparam.xsize, cparam.ysize);
        arParamChangeSize(&cparam, camSize.x, camSize.y, &cparam);
    }
    
    if ((gCparamLT= arParamLTCreate(&cparam, AR_PARAM_LT_DEFAULT_OFFSET)) == NULL) {
        ARLOGe("setupCamera(): Error: arParamLTCreate.\n");
        return false;
    }
    
    cvColorFrame.allocate(camSize.x, camSize.y);
    
    return true;
}

//UPDATE------------
void NftTracker::update(ARUint8 * arPix){
    
    updateMarkerDetection(arPix);
    
    updateMarkers();
}

void NftTracker::updateMarkerDetection(ARUint8 *arPix){
    if(threadHandle){
        float err;
        int ret;
        int pageNo;
        
        if(detectedPage==-2){
            trackingInitStart(threadHandle, arPix);
            detectedPage = int(PAGE_INITED_OK);
        }else if(detectedPage==-1){
            ret=trackingInitGetResult(threadHandle, trackingTrans, &pageNo);
            if(ret==1){
                if(pageNo>=0&&pageNo<surfaceSetCount){
                    ARLOGd("Detected page %d\n",pageNo);
                    detectedPage=pageNo;
                    ar2SetInitTrans(surfaceSet[detectedPage], trackingTrans);
                }else{
                    ARLOGe("Detected BAD page %d\n",pageNo);
                    detectedPage=int(PAGE_NOT_INITED);
                }
            }else if(ret<0){
                ARLOGd("NO page detected\n");
                detectedPage=int(PAGE_NOT_INITED);
            }
        }else if(detectedPage>=0 && detectedPage<surfaceSetCount){
            int ee = ar2Tracking(ar2Handle, surfaceSet[detectedPage], arPix, trackingTrans, &err);
            if(ee<0){
                ARLOGd("Tracking lost: %d\n",ee);
                detectedPage=int(PAGE_NOT_INITED);
            }else{
                ARLOGd("Tracked page %d (max %d)\n",detectedPage,surfaceSetCount-1);
            }
        }
    }else{
        ARLOGe("Error in ARToolkit threadHandle\n");
        detectedPage=int(PAGE_NOT_INITED);
    }
}

void NftTracker::updateMarkers(){
    for(int i=0;i<markersNFTCount;i++){
        markersNFT[i].validPrev = markersNFT[i].valid;
        if(markersNFT[i].pageNo>=0 && markersNFT[i].pageNo==detectedPage){
            markersNFT[i].valid=true;
            for(int j=0;j<3;j++){
                for(int k=0;k<4;k++){
                    markersNFT[i].trans[j][k]=trackingTrans[j][k];
                }
            }
        }else{
            markersNFT[i].valid=false;
        }
        
        if(markersNFT[i].valid){
            //filter pose estimate
            if(markersNFT[i].ftmi){
                if(arFilterTransMat(markersNFT[i].ftmi, markersNFT[i].trans, !markersNFT[i].validPrev)<0){
                    ARLOGe("ERROR: arFilterTransMat error with marker %d\n",i);
                }
            }
            
            if(!markersNFT[i].validPrev){
                //new marker!
                bFound = true;
                selectedId=i;
                ofNotifyEvent(evNewMarker, i);
            }
            
            //new pose, set it
            arglCameraViewRH(markersNFT[i].trans, markersNFT[i].pose.T, viewScaleFactor);
        }else{
            if(markersNFT[i].validPrev){
                //marker lost!
                bFound = false;
                selectedId=-1;
                ofNotifyEvent(evLostMarker, i);
            }
        }
        
    }
}

//DRAW----------

void NftTracker::drawDebug(){
    ofPushStyle();
    if(isFound()){
        ofSetColor(ofColor::green);
        ofDrawBitmapString("Marker FOUND!", 10, 10);
    }else{
        ofSetColor(ofColor::red);
        ofDrawBitmapString("Marker not found", 10, 10);
    }
    ofPopStyle();
}

//MATRICES-----------
void NftTracker::beginAR(){
    ofRectangle r(0,0,viewportSize.x,viewportSize.y);
    beginAR(r);
}

void NftTracker::beginAR(ofRectangle & rViewport){
    ofPushView();
    ofViewport(rViewport);
    loadProjectionMatrix();
    loadModelMatrix();
}

void NftTracker::endAR(){
    ofPopView();
}

void NftTracker::loadProjectionMatrix(){
    glMatrixMode(GL_PROJECTION);
#ifdef ARDOUBLE_IS_FLOAT
	glLoadMatrixf(cameraLens);
#else
	glLoadMatrixd(cameraLens);
#endif
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void NftTracker::loadModelMatrix(){
    loadModelMatrix(&getSelectedMarker());
}

void NftTracker::loadModelMatrix(ARMarkerNFT *mk){
    if(mk->valid){
#ifdef ARDOUBLE_IS_FLOAT
        glLoadMatrixf(mk->pose.T);
#else
        glLoadMatrixd(mk->pose.T);
#endif
    }
}
