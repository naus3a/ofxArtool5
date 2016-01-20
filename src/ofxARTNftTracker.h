//
//  ofxARTNftTracker.h
//  artool5example
//
//  Created by nausea on 12/23/15.
//
//

#pragma once
#include "ofxARTGenericTracker.h"

#include <AR/gsub_lite.h>
#include <AR/arMulti.h>
#include <AR/arFilterTransMat.h>
#include <AR2/tracking.h>
#include <AR2/util.h>
#include <KPM/kpm.h>
#include "ARMarkerNFT.h"
#include "trackingSub.h"
#include "ofxARTNftUtils.h"

#define PAGES_MAX 10

namespace ofxArtool5 {
    
    enum PageStatus{
        PAGE_NOT_INITED = -2,
        PAGE_INITED_OK = -1
    };
    
    class NftTracker : public GenericTracker{
    public:
        NftTracker();
        ~NftTracker();
        void nullArtPointers();
        void cleanup();
        
        bool setup(ofVec2f _camSize, ofVec2f _viewportSize, ofPixelFormat pf=OF_PIXELS_BGR, string pthCamParam="ARTdata/camera_para.dat", string pthMarkerData="ARTdata/markers.dat", string pthCustomDat="ARTdata/custom.dat");
        
        void update(ARUint8 * arPix);
        template<class T> inline void update(ofBaseHasPixels_<T> & ofImg){GenericTracker::update(ofImg);}
        
        //MATRICES
        void beginAR(ofRectangle & rViewport);
        void beginAR();
        void endAR();
        void loadProjectionMatrix();
        void loadModelMatrix();
        void loadModelMatrix(ARMarkerNFT * mk);
        
        //MARKERS
        int getNumMarkers(){return markersNFTCount;}
        ARMarkerNFT * getMarkers(){return markersNFT;}
        ARMarkerNFT & getMarker(int id){return getMarkers()[id];}
        ARMarkerNFT & getSelectedMarker(){return getMarker(selectedId);}
        
        void drawDebug();
        
        //GETTERS
        bool isFound(){return bFound;}
        
        ofEvent<int> evNewMarker;
        ofEvent<int> evLostMarker;
        
    protected:
        
        bool loadNFTData(string pthCustomDat);
        int unloadNFTData();
        bool initNFT(ARParamLT * cparamLT, AR_PIXEL_FORMAT pixFormat);
    
        //SETUP
        bool setupCamera(string pthCamParam, ofVec2f _camSize, ofVec2f _viewportSize, ofPixelFormat pf=OF_PIXELS_RGB);
        
        void updateMarkerDetection(ARUint8 * arPix);
        void updateMarkers();
        
        KpmHandle * kpmHandle;
        AR2HandleT * ar2Handle;
        ARMarkerNFT * markersNFT;
        THREAD_HANDLE_T * threadHandle;
        AR2SurfaceSetT * surfaceSet[PAGES_MAX];
        
        ARGL_CONTEXT_SETTINGS_REF gArglSettings;
        
        ARdouble cameraLens[16];
        
        float trackingTrans[3][4];
        
        int markersNFTCount;
        int surfaceSetCount;
        
        int selectedId;
        int detectedPage;
        
        bool bFound;
    };
}