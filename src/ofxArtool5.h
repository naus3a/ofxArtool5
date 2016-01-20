//
//  ofxArtool5.h
//  artool5example
//
//  Created by nausea on 12/9/15.
//
//

#pragma once
#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxArtoolMode.h"
#include "ofxArtoolModePatt.h"

/*#include <AR/config.h>
#include <AR/video.h>
#include <AR/param.h>			// arParamDisp()
#include <AR/ar.h>
#include <AR/gsub_lite.h>

#include <AR/arMulti.h>
#include <AR/arFilterTransMat.h>
#include <AR2/tracking.h>
#include <KPM/kpm.h>
#include "ARMarkerNFT.h"
#include "trackingSub.h"*/

#define PAGES_MAX 10

class ofxArtool5{
public:
    
    enum ARToolKitMode{
        ART_PATTERN,
        ART_NFT
    };
    
    ofxArtool5();
    ~ofxArtool5();
    void nullArtPointers();
    void cleanup();
    bool setup(ofVec2f _camSize, ofVec2f _viewportSize);
    bool setup(ofVec2f _camSize, ofVec2f _viewportSize, ARToolKitMode _mode);
    bool setupPattern(ofVec2f _camSize, ofVec2f _viewportSize, ofPixelFormat pf=OF_PIXELS_BGR, string pthCamParam="ARTdata/camera_para.dat", string pthMarker="ARTdata/patt.hiro");
    bool setupNFT(ofVec2f _camSize, ofVec2f _viewportSize, ofPixelFormat pf=OF_PIXELS_BGR, string pthCamParam="ARTdata/camera_para.dat", string pthMarkerData="ARTdata/markers.dat");
    
    void update(ofPixels & pix);
    template<class T> inline void update(ofBaseHasPixels_<T> & ofImg){update(ofImg.getPixels());}
    void update(ARUint8 * arPix);
    
    void updatePattern(ARUint8 * arPix);
    
    void drawDebug();
    
    //matrices
    inline void getARProjectionMatrix(ARdouble * mat){arglCameraFrustumRH(&(gCparamLT->param), getMinDistance(), getMaxDistance(), mat);}
    inline void getARModelMatrix(ARdouble * mat){arglCameraViewRH(gPatt_trans, mat, getScaleFactor());}
    void loadProjectionMatrix();
    void loadModelMatrix();
    void beginAR();
    void endAR();
    
    //misc getters
    inline bool isFound(){return bPattFound;}
    inline float getScaleFactor(){return viewScaleFactor;}
    inline float getMinDistance(){return viewDistanceMin;}
    inline float getMaxDistance(){return viewDistanceMax;}
    inline double getPatternWidth(){return double(gPatt_width);}
    inline AR_LABELING_THRESH_MODE getThreshMode(){
        AR_LABELING_THRESH_MODE tm;
        arGetLabelingThreshMode(gARHandle, &tm);
        return tm;
    }
    string getThresholdModeName(AR_LABELING_THRESH_MODE tm);
    int getThreshold();
    int getImageProcMode(){
        int md;
        arGetImageProcMode(gARHandle, &md);
        return md;
    }
    string getImageProcModeName(int md);
    inline ARToolKitMode getMode(){return artMode;}
    //marker getters
    inline ARMarkerInfo * getMarkers(){return gARHandle->markerInfo;}
    inline ARMarkerInfo & getMarker(int id){return getMarkers()[id];}
    inline ARMarkerInfo & getSelectedMarker(){return getMarker(selectedId);}
    inline int getMarkerDir(ARMarkerInfo & marker){return marker.dir;}
    inline ofVec2f getMarkerCenter(ARMarkerInfo & _marker){return ofVec2f(_marker.pos[0],_marker.pos[1]);}
    inline ofVec2f getMarker2DPoint(ARMarkerInfo & _marker, int pid){return ofVec2f(_marker.vertex[pid][0],_marker.vertex[pid][1]);}
    vector<ofVec2f> getMarker2DCorners(ARMarkerInfo & marker);
    vector<ofVec2f> getMarker2DOrderedCorners(ARMarkerInfo & info);
    
    //misc setters
    inline void setScaleFactor(float f){viewScaleFactor=f;}
    inline void setMinDistance(float f){viewDistanceMin=f;}
    inline void setMaxDistance(float f){viewDistanceMax=f;}
    inline void setPatternWidth(double pw){gPatt_width=ARdouble(pw);}
    void setThresholdMode(AR_LABELING_THRESH_MODE tm){arSetLabelingThreshMode(gARHandle, tm);}
    void setThreshold(int thr);
    void setImageProcMode(int md);
    
    //utils
    //static AR_PIXEL_FORMAT toAR(ofPixelFormat &ofPf);
    //template<class T> inline ARUint8 * toAR(ofBaseHasPixels_<T> & img){
    //    return const_cast<unsigned char*>(img.getPixels().getData());
    //}
    
    ofEvent<ARUint8 *> evARUpdate;
protected:
    ofxArtoolMode * curArtMode;
    
    ARParamLT * gCparamLT;
    
    ARHandle * gARHandle;
    AR3DHandle * gAR3DHandle;
    ARPattHandle * gARPattHandle;
    
    KpmHandle * kpmHandle;
    AR2HandleT * ar2Handle;
    ARMarkerNFT * markersNFT;
    THREAD_HANDLE_T * threadHandle;
    AR2SurfaceSetT * surfaceSet[PAGES_MAX];

    ARdouble gPatt_width;
    ARdouble gPatt_trans[3][4];

    ARdouble p[16];
    ARdouble m[16];
    
    ARdouble cameraLens[16];
    
    bool setupCamera(string pthCamParam, ofVec2f _camSize, ofVec2f _viewportSize, ofPixelFormat pf=OF_PIXELS_RGB);
    bool setupMarker(string pthMarker);
    bool initNFT(ARParamLT * cparamLT, AR_PIXEL_FORMAT pixFormat);
    void unloadNFTData();
    
    int getHighConfMarker();
    void calcMarkerTransformation(int & mId);
    
    ofxCvColorImage cvColorFrame;
    ofxCvGrayscaleImage cvGrayFrame;
    
    vector<string> threshNames;
    
    ofVec2f camSize;
    ofVec2f viewportSize;
    float viewScaleFactor;
    float viewDistanceMin;
    float viewDistanceMax;
    int pattId;
    int selectedId;
    int markersNFTCount;
    int surfaceSetCount;
    ARToolKitMode artMode;
    bool bPattFound;
};