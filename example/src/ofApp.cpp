#include "ofApp.h"
#include "ofxXmlSettings.h"

//--------------------------------------------------------------
void ofApp::setup(){
	string ip = "192.168.0.10";

	ofxXmlSettings xml;
	bool ret = xml.load("settings.xml");
	ofxURGTouchConfig settings;
	if (ret) {
		ip = xml.getValue("settings:ip", ip);
	}
	_sensor.setupEthernet(0, ip, settings);

	ofAddListener(_sensor.TouchBegan, this, &ofApp::onTouchBegan);
	ofAddListener(_sensor.TouchMoved, this, &ofApp::onTouchMoved);
	ofAddListener(_sensor.TouchEnded, this, &ofApp::onTouchEnded);

	ofSetFrameRate(60);
}

//--------------------------------------------------------------
void ofApp::update(){
	_sensor.update();
}

void ofApp::exit() {
	_sensor.exit();
}

//--------------------------------------------------------------
void ofApp::draw(){
	//_sensor.draw();
	ofBackground(0);
	for (int i = 0; i < _touches.size(); i++) {
		ofSetColor(_colors[i]);
		ofDrawCircle(_touches[i].x * ofGetWidth(), _touches[i].y * ofGetHeight(), 10);
		ofSetColor(255);
		ofDrawBitmapString(_touches[i].id, _touches[i].x, _touches[i].y);
	}

	ofDrawBitmapString(ofGetFrameRate(), 4, 16);
	ofDrawBitmapString(_sensor.updateMillis, 4, 32);
}

void ofApp::onTouchBegan(ofxURGTouch& touch) {
	ofColor c = ofColor::fromHsb(ofRandom(255), 255, 255);
	_touches.push_back(touch);
	_colors.push_back(c);
	cout << "touchstart " << touch.id << endl;
}

void ofApp::onTouchMoved(ofxURGTouch& touch) {
	for (int i = 0; i < _touches.size(); i++) {
		if (touch.id == _touches[i].id) {
			_touches[i] = touch;
			break;
		}
	}
}

void ofApp::onTouchEnded(ofxURGTouch& touch) {
	for (int i = _touches.size() - 1; 0 <= i; i--) {
		if (touch.id == _touches[i].id) {
			_touches.erase(_touches.begin() + i);
			_colors.erase(_colors.begin() + i);
			break;
		}
	}
	cout << "touchend " << touch.id << endl;
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
	ofxURGTouch touch;
	touch.x = x;
	touch.y = y;
	touch.id = _mouseId;
	ofNotifyEvent(_sensor.TouchMoved, touch);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	ofxURGTouch touch;
	touch.x = x;
	touch.y = y;
	touch.id = ++_mouseId;
	ofNotifyEvent(_sensor.TouchBegan, touch);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	ofxURGTouch touch;
	touch.x = x;
	touch.y = y;
	touch.id = _mouseId;
	ofNotifyEvent(_sensor.TouchEnded, touch);
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
