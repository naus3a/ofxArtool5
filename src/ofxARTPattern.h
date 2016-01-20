//
//  ofxARTPattern.h
//
//  Created by nausea on 12/23/15.
//
//

#pragma once
#include "ofxARTGenericTracker.h"

#include <AR/config.h>
#include <AR/video.h>
#include <AR/ar.h>
#include <AR/gsub_lite.h>

namespace ofxArtool5 {
    class PatternTracker : public GenericTracker{
    public:
        PatternTracker();
        ~PatternTracker();
        void nullArtPointers();
        void cleanup();
        
        bool setup(ofVec2f _camSize, ofVec2f _viewportSize, ofPixelFormat pf=OF_PIXELS_BGR, string pthCamParam="ARTdata/camera_para.dat", string pthMarker="ARTdata/patt.hiro");
        
        
        void update(ARUint8 * arPix);
        //void update(ofPixels & pix);
        template<class T> inline void update(ofBaseHasPixels_<T> & ofImg){GenericTracker::update(ofImg);}
        
        void drawDebug();
        
        //MATRICES
        void loadProjectionMatrix();
        void loadModelMatrix();
        void beginAR(ofRectangle & rViewport);
        void beginAR();
        void endAR();
        
        //MARKER
        inline int getNumMarkers(){return gARHandle->marker_num;}
        inline ARMarkerInfo * getMarkers(){return gARHandle->markerInfo;}
        inline ARMarkerInfo & getMarker(int id){return getMarkers()[id];}
        inline ARMarkerInfo & getSelectedMarker(){return getMarker(selectedId);}
        
        //GETTERS
        inline bool isFound(){return bPattFound;}
        //inline float getScaleFactor(){return viewScaleFactor;}
        //inline float getMinDistance(){return viewDistanceMin;}
        //inline float getMaxDistance(){return viewDistanceMax;}
        inline double getPatternWidth(){return double(gPatt_width);}
        
        inline AR_LABELING_THRESH_MODE getThreshMode(){
            AR_LABELING_THRESH_MODE tm;
            arGetLabelingThreshMode(gARHandle, &tm);
            return tm;
        }
        inline string getThreshModeName(){return getThresholdModeName(getThreshMode());}
        int getThreshold();
        inline int getImageProcMode(){
            int md;
            arGetImageProcMode(gARHandle, &md);
            return md;
        }
        
        //SETTERS
        //inline void setScaleFactor(float f){viewScaleFactor=f;}
        //inline void setMinDistance(float f){viewDistanceMin=f;}
        //inline void setMaxDistance(float f){viewDistanceMax=f;}
        inline void setPatternWidth(double pw){gPatt_width=ARdouble(pw);}
        
        inline void setThresholdMode(AR_LABELING_THRESH_MODE tm){arSetLabelingThreshMode(gARHandle, tm);}
        void setThreshold(int thr);
        void setImageProcMode(int md);
        
    protected:
        //SETUP
        bool setupCamera(string pthCamParam, ofVec2f _camSize, ofVec2f _viewportSize, ofPixelFormat pf=OF_PIXELS_RGB);
        bool setupMarker(string pthMarker);
        
        //MATRICES
        inline void getARProjectionMatrix(ARdouble * mat){arglCameraFrustumRH(&(gCparamLT->param), getMinDistance(), getMaxDistance(), mat);}
        inline void getARModelMatrix(ARdouble * mat){arglCameraViewRH(gPatt_trans, mat, getScaleFactor());}
        
        //MARKER
        int getHighConfMarker();
        void calcMarkerTransformation(int & mId);
        
        //ofxCvColorImage cvColorFrame;
        
        ARHandle * gARHandle;
        AR3DHandle * gAR3DHandle;
        ARPattHandle * gARPattHandle;
        
        ARdouble gPatt_width;
        ARdouble gPatt_trans[3][4];
        
        ARdouble p[16];
        ARdouble m[16];
        
        //float viewScaleFactor;
        //float viewDistanceMin;
        //float viewDistanceMax;
        
        int pattId;
        int selectedId;
        bool bPattFound;
    };
}