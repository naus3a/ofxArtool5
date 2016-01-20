//
//  ofxARTGenericTracker.h
//
//  Created by nausea on 12/23/15.
//
//

#pragma once
#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxARTUtils.h"

#include <AR/param.h>

namespace ofxArtool5 {
    class GenericTracker{
    public:
        virtual void nullArtPointers()=0;
        virtual void cleanup()=0;
        
        virtual void update(ARUint8 * arPix)=0;
        virtual void update(ofPixels & pix){
            cvColorFrame.setFromPixels(pix);
            ARUint8 * arImg = toAR(cvColorFrame);
            update(arImg);
        }
        
        template<class T> void update(ofBaseHasPixels_<T> &ofImg){update(ofImg.getPixels());}
        
        virtual void drawDebug()=0;
        
        //GETTERS
        virtual bool isFound()=0;
        inline float getScaleFactor(){return viewScaleFactor;}
        virtual inline float getMinDistance(){return viewDistanceMin;}
        virtual inline float getMaxDistance(){return viewDistanceMax;}
        
        //SETTERS
        inline void setScaleFactor(float f){viewScaleFactor=f;}
        virtual inline void setMinDistance(float f){viewDistanceMin=f;}
        virtual inline void setMaxDistance(float f){viewDistanceMax=f;}
        
    protected:
        ARParamLT * gCparamLT;
        
        ofxCvColorImage cvColorFrame;
        
        ofVec2f camSize;
        ofVec2f viewportSize;
        
        float viewScaleFactor;
        float viewDistanceMin;
        float viewDistanceMax;
    };
}