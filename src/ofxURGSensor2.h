#pragma once

#include "Urg_driver.h"
#include "math_utilities.h"
#include "ofMain.h"

#ifdef DEBUG
#pragma comment(lib, "urg_cpp_debug.lib")
#else
#pragma comment(lib, "urg_cpp.lib")
#endif

using namespace qrk;

struct ofxURGObject {
	double beginRad, endRad, shortestRad;
	double shortestLength = 10000;
};

struct ofxURGTouch {
	int sensorId, id;
	float x, y;
	long lastUpdate;
};

struct ofxURGTouchConfig {
	float touchDistTh = 0.05;
	int touchDownDelay = 250;
	int touchUpDelay = 250;
	float movingWeight = 0.5;

	int sensorLeft = 1600;
	int sensorRight = -1600;
	int sensorTop = 150;
	int sensorBottom = 2000;

	int screenWidth = 3840;
	int screenHeight = 2160;

	int screenLeft = 0;
	int screenRight = 3840;
	int screenTop = 0;
	int screenBottom = 2160;
};

class ofxURGSensor2 {
public:
	void setupEthernet(int id, string ip, ofxURGTouchConfig settings, int skipFrame = 0);
	void setupUSB(int id, string port, int bps, ofxURGTouchConfig settings, int skipFrame = 0);
	void update();
	void exit();
	bool isInitialized();
	int getId() { return _id; }
	string getAddress() { return _address; }
	vector<ofxURGTouch> getTouches();

	ofEvent<ofxURGTouch> TouchBegan, TouchMoved, TouchEnded;
	float updateMillis;

	class SensorThread : public ofThread {
	public:
		void setupUSB(ofxURGSensor2* parent, string port, int bps, int skipFrame = 0);
		void setupEthernet(ofxURGSensor2* parent, string ip, int skipFrame = 0);
		void requestRestart();
	private:
		void threadedFunction();
		void updateTouches(const vector<ofxURGObject>& objects);

		Urg_driver _urg;
		int _skipFrame;

		bool _objectInRange = false, _needsRestart = false;
		 int _lastTouchId;

		vector<long> _data;
		vector<ofxURGTouch> _candidates;
		
		vector<int> _updateMillis;
		int _updateMillisCount = 5;

		ofxURGSensor2* _parent;
	};

private:
	vector<ofxURGTouch> _touches, _beganTouches, _movedTouches, _endedTouches;
	SensorThread _thread;

	bool _initialized;
	int _id;
	string _address;
	ofxURGTouchConfig _settings;

};


