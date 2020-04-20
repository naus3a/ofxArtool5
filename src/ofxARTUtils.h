//
//  ofxARTUtils.h
//
//  Created by nausea on 12/23/15.
//
//

#pragma once
#include "ofMain.h"

#include <AR/config.h>
#include <AR/ar.h>

namespace ofxArtool5 {
    //toAR
    inline AR_PIXEL_FORMAT toAR(ofPixelFormat & ofPf){
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
    
    template<class T> inline ARUint8 * toAR(ofBaseHasPixels_<T> & img){
        return const_cast<unsigned char*>(img.getPixels().getData());
    }
    
    //MARKER
    inline int getMarkerDir(ARMarkerInfo & mk){return mk.dir;}
    inline ofVec2f getMarkerCenter(ARMarkerInfo & mk){return ofVec2f(mk.pos[0],mk.pos[1]);}
    inline ofVec2f getMarker2DPoint(ARMarkerInfo & mk, int pid){return ofVec2f(mk.vertex[pid][0],mk.vertex[pid][1]);}
    inline vector<ofVec2f> getMarker2DCorners(ARMarkerInfo &mk){
        vector<ofVec2f> crn;
        crn.clear();
        for(int i=0;i<4;i++){
            crn.push_back(getMarker2DPoint(mk, i));
        }
        return crn;
    }
    inline vector<ofVec2f> getMarker2DOrderedCorners(ARMarkerInfo & mk){
        vector<ofVec2f> crn;
        crn.clear();
        int index = 4 - mk.dir;
        for(int i=0;i<4;i++){
            crn.push_back(getMarker2DPoint(mk, index));
            index++;
            if(index>=4)index=0;
        }
        return crn;
    }
    
    //MISC
    inline vector<string> getThresholdNames(){
        vector<string> threshNames;
        threshNames.resize(6);
        threshNames[0] = "MANUAL";
        threshNames[1] = "AUTO MEDIAN";
        threshNames[2] = "AUTO OTSU";
        threshNames[3] = "AUTO ADAPTIVE";
        threshNames[4] = "AUTO BRACKETING";
        threshNames[5] = "UNKNOWN";
        return threshNames;
    }
    
    inline string getThresholdModeName(AR_LABELING_THRESH_MODE tm){
        string s;
        switch(tm){
            case AR_LABELING_THRESH_MODE_MANUAL:
                s = "MANUAL";
                break;
            case AR_LABELING_THRESH_MODE_AUTO_MEDIAN:
                s = "AUTO MEDIAN";
                break;
            case AR_LABELING_THRESH_MODE_AUTO_OTSU:
                s = "AUTO OTSU";
                break;
            case AR_LABELING_THRESH_MODE_AUTO_ADAPTIVE:
                s = "AUTO ADAPTIVE";
                break;
            case AR_LABELING_THRESH_MODE_AUTO_BRACKETING:
                s = "AUTO BRACKETING";
                break;
            default:
                s = "UNKNOWN";
                break;
        }
        return s;
    }
    
    inline string getImageProcModeName(int md){
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

}
