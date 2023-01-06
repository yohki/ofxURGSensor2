#include "ofxURGSensor2.h"
#include "ofEvents.h"

void ofxURGSensor2::setupEthernet(int id, string ip, ofxURGTouchConfig settings, int skipFrame) {
	_id = id;
	_address = ip;
	_thread.setupEthernet(this, ip, skipFrame);
	_thread.startThread(true);
	_settings = settings;
}

void ofxURGSensor2::setupUSB(int id, string port, int bps, ofxURGTouchConfig settings, int skipFrame) {
	_id = id;
	_address = port;
	_thread.setupUSB(this, port, bps, skipFrame);
	_thread.startThread(true);
	_settings = settings;
}

bool ofxURGSensor2::isInitialized() {
	return _initialized;
}

void ofxURGSensor2::update() {
	_thread.lock();
	for (int i = 0; i < _beganTouches.size(); i++) {
		_beganTouches[i].sensorId = _id;
		ofNotifyEvent(TouchBegan, _beganTouches[i]);
	}
	_beganTouches.clear();

	for (int i = 0; i < _movedTouches.size(); i++) {
		_movedTouches[i].sensorId = _id;
		ofNotifyEvent(TouchMoved, _movedTouches[i]);
	}
	_movedTouches.clear();

	for (int i = 0; i < _endedTouches.size(); i++) {
		_endedTouches[i].sensorId = _id;
		ofNotifyEvent(TouchEnded, _endedTouches[i]);
	}
	_endedTouches.clear();
	_thread.unlock();

	if (200 < updateMillis) {
		_thread.requestRestart();
		cout << "#" << _id << " Restart (update time " << updateMillis << "ms)" << endl;
	}
}

void ofxURGSensor2::exit() {
	if (_initialized) {
		_thread.waitForThread(true, 1000);
	}
}

void ofxURGSensor2::SensorThread::setupUSB(ofxURGSensor2* parent, string port, int bps, int skipFrame) {
	_parent = parent;
	_parent->_initialized = _urg.open(port.c_str(), bps, Urg_driver::Serial);

	if (!_parent->_initialized) {
		ofLogError("Urg_driver::open()", port);
		return;
	}

	int beginStep = _urg.rad2step(-PI / 2);
	int endStep = _urg.rad2step(PI / 2);
	_skipFrame = skipFrame;
	_urg.set_scanning_parameter(beginStep, endStep, _skipFrame);

	_urg.start_measurement(Urg_driver::Distance, 0, _skipFrame);
}

void ofxURGSensor2::SensorThread::setupEthernet(ofxURGSensor2* parent, string ip, int skipFrame) {
	_parent = parent;
	int port = 10940;
	_parent->_initialized = _urg.open(ip.c_str(), port, Urg_driver::Ethernet);
	
	if (!_parent->_initialized) {
		ofLogError("Urg_driver::open()", ip + ":" + ofToString(port));
		return;
	}

	int beginStep = _urg.rad2step(-PI / 2);
	int endStep = _urg.rad2step(PI / 2);
	_skipFrame = skipFrame;
	_urg.set_scanning_parameter(beginStep, endStep, _skipFrame);

	_urg.start_measurement(Urg_driver::Distance, 0, _skipFrame);
}

vector<ofxURGTouch> ofxURGSensor2::getTouches() {
	_thread.lock();
	vector<ofxURGTouch> touches = _touches;
	_thread.unlock();
	return touches;
}

void ofxURGSensor2::SensorThread::threadedFunction() {
	_lastTouchId = 0;
	while (isThreadRunning()) {
		if (_needsRestart) {
			_urg.stop_measurement();
			_urg.start_measurement(Urg_driver::Distance, 0, _skipFrame);
			_needsRestart = false;
			_updateMillis.clear();
		}

		long st = ofGetElapsedTimeMillis();
		long ts;
		bool ret = _urg.get_distance(_data, &ts);
		if (ret) {
			long min_distance = _urg.min_distance();
			long max_distance = _urg.max_distance();
			int n = _data.size();
			vector<ofxURGObject> objects;

			for (int i = 0; i < n; i++) {
				double l = static_cast<double>(_data[i]);
				if ((l <= min_distance) || (l >= max_distance)) {
					continue;
				}

				double rad = _urg.index2rad(i) + PI / 2;

				float x = static_cast<float>(l * cos(rad));
				float y = static_cast<float>(l * sin(rad));

				if (_parent->_settings.sensorRight < x && x < _parent->_settings.sensorLeft &&
					_parent->_settings.sensorTop < y && y < _parent->_settings.sensorBottom) {
					// Something is there
					if (_objectInRange) {
						// update data and find the shortest point
						ofxURGObject object = objects[objects.size() - 1];
						object.endRad = rad;
						if (l < object.shortestLength) {
							object.shortestLength = l;
							object.shortestRad = rad;
						}
						objects[objects.size() - 1] = object;
					} else {
						// create an object

						ofxURGObject object;
						object.beginRad = rad;
						object.endRad = rad;
						object.shortestRad = rad;
						object.shortestLength = l;
						objects.push_back(object);

						_objectInRange = true;
					}
				} else {
					_objectInRange = false;
				}
			}
			updateTouches(objects);
		}
		// Calculate update time
		long et = ofGetElapsedTimeMillis();
		int ms = et - st;
		_updateMillis.push_back(ms);
		if (_updateMillisCount < _updateMillis.size()) {
			_updateMillis.erase(_updateMillis.begin());
		}
		_parent->updateMillis = 0;
		for (int i = 0; i < _updateMillis.size(); i++) {
			_parent->updateMillis += _updateMillis[i];
		}
		_parent->updateMillis /= _updateMillis.size();
	}
	_urg.stop_measurement();
	_urg.close();
}

void ofxURGSensor2::SensorThread::requestRestart() {
	_needsRestart = true;
}

void ofxURGSensor2::SensorThread::updateTouches(const vector<ofxURGObject>& objects) {
	vector<ofxURGTouch> candidates, beganTouches, movedTouches, endedTouches;
	long t = ofGetElapsedTimeMillis();

	float normLeft = 1.0 * _parent->_settings.screenLeft / _parent->_settings.screenWidth;
	float normRight = 1.0 * _parent->_settings.screenRight / _parent->_settings.screenWidth;
	float normTop = 1.0 * _parent->_settings.screenTop / _parent->_settings.screenHeight;
	float normBottom = 1.0 * _parent->_settings.screenBottom / _parent->_settings.screenHeight;

	for (int i = 0; i < objects.size(); i++) {
		ofxURGObject object = objects[i];
		float x = static_cast<float>(object.shortestLength * cos(object.shortestRad));
		float y = static_cast<float>(object.shortestLength * sin(object.shortestRad));

		ofPoint p;
		p.x = ofMap(x, _parent->_settings.sensorLeft, _parent->_settings.sensorRight, normLeft, normRight, true);
		p.y = ofMap(y, _parent->_settings.sensorTop, _parent->_settings.sensorBottom, normTop, normBottom, true);

		bool newTouch = true;

		for (int j = _candidates.size() - 1; 0 <= j; j--) {
			ofxURGTouch cand = _candidates[j];
			float distSq = p.distanceSquared(ofPoint(cand.x, cand.y));
			if (distSq < _parent->_settings.touchDistTh * _parent->_settings.touchDistTh) {
				if (_parent->_settings.touchDownDelay < t - cand.lastUpdate) {
					cand.lastUpdate = t;
					// TOUCH DOWN
					beganTouches.push_back(cand);
					newTouch = false;
				} else {
					candidates.push_back(cand);
				}
				break;
			}
		}

		if (newTouch) {
			for (int j = 0; j < beganTouches.size(); j++) {
				ofxURGTouch touch = beganTouches[j];
				if (p.distanceSquared(ofPoint(touch.x, touch.y)) < _parent->_settings.touchDistTh * _parent->_settings.touchDistTh) {
					newTouch = false;
					break;
				}
			}
		}

		if (newTouch) {
			for (int j = 0; j < movedTouches.size(); j++) {
				ofxURGTouch touch = movedTouches[j];
				if (p.distanceSquared(ofPoint(touch.x, touch.y)) < _parent->_settings.touchDistTh * _parent->_settings.touchDistTh) {
					newTouch = false;
					break;
				}
			}
		}

		if (newTouch) {
			lock();
			for (int j = _parent->_touches.size() - 1; 0 <= j; j--) {
				ofxURGTouch touch = _parent->_touches[j];
				if (p.distanceSquared(ofPoint(touch.x, touch.y)) < _parent->_settings.touchDistTh * _parent->_settings.touchDistTh) {
					touch.x = touch.x * (1.0 - _parent->_settings.movingWeight) + p.x * _parent->_settings.movingWeight;
					touch.y = touch.y * (1.0 - _parent->_settings.movingWeight) + p.y * _parent->_settings.movingWeight;
					touch.lastUpdate = t;
					// TOUCH MOVED
					movedTouches.push_back(touch);
					_parent->_touches.erase(_parent->_touches.begin() + j);
					newTouch = false;
					break;
				}
			}
			unlock();
		}

		if (newTouch) {
			bool add = true;
			for (int j = 0; j < candidates.size(); j++) {
				ofxURGTouch cand = candidates[j];
				if (p.distanceSquared(ofPoint(cand.x, cand.y)) < _parent->_settings.touchDistTh * _parent->_settings.touchDistTh) {
					add = false;
					break;
				}
			}

			if (add) {
				ofxURGTouch touch;
				touch.id = ++_lastTouchId;
				touch.x = p.x;
				touch.y = p.y;
				touch.lastUpdate = t;
				candidates.push_back(touch);
			}
			//cout << x << ", " << y << endl;
		}
	}
	_candidates = candidates;

	vector<ofxURGTouch> lostTouches;

	lock();
	for (int i = 0; i < _parent->_touches.size(); i++) {
		// TOUCH ENDED
		int dt = t - _parent->_touches[i].lastUpdate;
		if (_parent->_settings.touchUpDelay < dt) {
			endedTouches.push_back(_parent->_touches[i]);
		} else {
			lostTouches.push_back(_parent->_touches[i]);
		}
	}

	_parent->_beganTouches = beganTouches;
	_parent->_movedTouches = movedTouches;
	_parent->_endedTouches = endedTouches;

	_parent->_touches.clear();
	_parent->_touches = beganTouches;
	_parent->_touches.insert(_parent->_touches.end(), movedTouches.begin(), movedTouches.end());
	_parent->_touches.insert(_parent->_touches.end(), lostTouches.begin(), lostTouches.end());

	unlock();
}
