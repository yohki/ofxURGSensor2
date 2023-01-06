#pragma once

#include "ofMain.h"
#include "ofxURGSensor2.h"

class ofApp : public ofBaseApp{

public:
	void setup();
	void update();
	void draw();
	void exit();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	void onTouchBegan(ofxURGTouch& touch);
	void onTouchMoved(ofxURGTouch& touch);
	void onTouchEnded(ofxURGTouch& touch);
private:
	ofxURGSensor2 _sensor;
	vector<ofxURGTouch> _touches;
	vector<ofColor> _colors;

	int _mouseId = 100000;
};
