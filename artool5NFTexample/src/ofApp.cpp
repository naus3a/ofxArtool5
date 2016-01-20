#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(0, 0, 0);
    ofSetFrameRate(60);
    
    cam.initGrabber(640, 480);
    art.setup(ofVec2f(640,480),ofVec2f(640,480));
    
    ofAddListener(art.evNewMarker, this, &ofApp::onNewMarker);
    ofAddListener(art.evLostMarker, this, &ofApp::onLostMarker);
}

//--------------------------------------------------------------
void ofApp::update(){
    cam.update();
    if(cam.isFrameNew()){
        art.update(cam);
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    cam.draw(0, 0);
    
    if(art.isFound()){
        art.beginAR();
        ofDrawLine(-1000, 0, 1000, 0);
        ofDrawLine(0, -1000, 0, 1000);
        ofDrawBox(0, 0, 0.5, 21, 21, 21);
        art.endAR();
    }
    
    art.drawDebug();
}

void ofApp::onNewMarker(int & mId){
    cout<<"New Marker found!"<<endl;
}

void ofApp::onLostMarker(int & mId){
    cout<<"Marker lost!"<<endl;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
