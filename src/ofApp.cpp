#include "ofApp.h"

using namespace cv;
using namespace ofxCv;

void ofApp::setup() {
    settings.loadFile("settings.xml");
    ofHideCursor();
    
    fbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
    
    videoColor = (bool) settings.getValue("settings:video_color", 1);
    drawWireframe = (bool) settings.getValue("settings:draw_wireframe", 0);
    doSpread = (bool) settings.getValue("settings:do_spread", 0);
    playLatk = (bool) settings.getValue("settings:play_latk", 1);
    fboRotation = settings.getValue("settings:fbo_rotation", 180);
    secondaryOscSend = (bool) settings.getValue("settings:secondary_osc_send", 0);

    oscilloscopeMode = (bool) settings.getValue("settings:oscilloscope_mode", 0);
    
    oscHost = settings.getValue("settings:osc_host", "127.0.0.1");
    oscSendPort = settings.getValue("settings:osc_send_port", 7110);
    oscReceivePort = settings.getValue("settings:osc_receive_port", 7111);
    setupOscSender(sender, oscHost, oscSendPort);
    setupOscReceiver(receiver, oscReceivePort);
    
    hostName = getHostName();
    sessionId = getSessionId();

    fgMesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
    
    if (playLatk) {
#ifdef TARGET_OPENGLES
        if (ofIsGLProgrammableRenderer()) {
            shader.load("vhsc_es3");
            std::cout << "* * * Using OpenGL ES3 * * *" << endl;
        } else {
            shader.load("vhsc_es2");
            std::cout << "* * * Using OpenGL ES2 * * *" << endl;
        }
#else
        if (ofIsGLProgrammableRenderer()) {
            shader.load("vhsc_gl3");
            std::cout << "* * * Using OpenGL 3 * * *" << endl;
        } else {
            shader.load("vhsc_gl2");
            std::cout << "* * * Using OpenGL 2 * * *" << endl;
        }
#endif
    }
    
    
    if (playLatk) {
        latkFileName = settings.getValue("settings:latk_file_name", "untitled.json");
        soundFileName = settings.getValue("settings:sound_file_name", "sound.mp3");
        latk = Latk(latkFileName);
        
        snd.load(soundFileName);
        snd.setLoop(true);
        snd.play();
    }
    
    startTimesArray.push_back(0.641); // 1. This place is a message, and part of a system of messages.
    startTimesArray.push_back(5.986); // 2. Pay attention to it!
    startTimesArray.push_back(8.053); // 3. Sending this message was important to us.
    startTimesArray.push_back(11.758); // 4. We considered ourselves to be a powerful culture.
    startTimesArray.push_back(15.322); // 5. This place is not a place of honor.
    startTimesArray.push_back(19.241); // 6. No highly esteemed deed is commemorated here.
    startTimesArray.push_back(23.446); // 7. Nothing valued is here.
    startTimesArray.push_back(26.225); // 8. What is here was dangerous and repulsive to us.
    startTimesArray.push_back(30.573); // 9. This message is a warning about danger.
    startTimesArray.push_back(33.637); // 10. The danger is in a particular location.
    startTimesArray.push_back(36.488); // 11. It increases towards a center.
    startTimesArray.push_back(38.982); // 12. The center of danger is here, of a particular size and shape, and below us.
    startTimesArray.push_back(45.040); // 13. The danger is still present, in your time, as it was in ours.
    startTimesArray.push_back(49.886); // 14. The danger is to the body, and it can kill.
    startTimesArray.push_back(53.734); // 15. The form of the danger is an emanation of energy.
    startTimesArray.push_back(57.226); // 16. The danger is unleashed only if you substantially disturb this place physically.
    startTimesArray.push_back(63.569); // 17. This place is best shunned and left uninhabited.
    
    stopTimesArray.push_back(5.487);
    stopTimesArray.push_back(7.340);
    stopTimesArray.push_back(11.046);
    stopTimesArray.push_back(14.538);
    stopTimesArray.push_back(18.244);
    stopTimesArray.push_back(22.876);
    stopTimesArray.push_back(25.299);
    stopTimesArray.push_back(29.646);
    stopTimesArray.push_back(32.996);
    stopTimesArray.push_back(36.060);
    stopTimesArray.push_back(38.412);
    stopTimesArray.push_back(44.403);
    stopTimesArray.push_back(48.817);
    stopTimesArray.push_back(52.451);
    stopTimesArray.push_back(56.442);
    stopTimesArray.push_back(62.357);
    stopTimesArray.push_back(66.918);
    
    for (int i=0; i<startTimesArray.size(); i++) {
        float newTimeDiff = abs(stopTimesArray[currentFrame] - startTimesArray[currentFrame]);
        if (newTimeDiff > largestTimeDiff) largestTimeDiff = newTimeDiff;
        diffTimesArray.push_back(newTimeDiff);
    }
    
    camW = settings.getValue("settings:cam_width", 640);
    camH = settings.getValue("settings:cam_height", 480);
    camFps = settings.getValue("settings:cam_fps", 40); // RPi cam can do this

    if (!playLatk) {
#ifdef TARGET_RASPBERRY_PI
        camRotation = settings.getValue("settings:cam_rotation", 0);
        camSharpness = settings.getValue("settings:sharpness", 0);
        camContrast = settings.getValue("settings:contrast", 0);
        camBrightness = settings.getValue("settings:brightness", 50);
        camIso = settings.getValue("settings:iso", 300);
        camExposureMode = settings.getValue("settings:exposure_mode", 0);
        camExposureCompensation = settings.getValue("settings:exposure_compensation", 0);
        camShutterSpeed = settings.getValue("settings:shutter_speed", 0);
        
        cam.setup(camW, camH, camFps, false); // color/gray;
        
        cam.setRotation(camRotation);
        cam.setSharpness(camSharpness);
        cam.setContrast(camContrast);
        cam.setBrightness(camBrightness);
        cam.setISO(camIso);
        cam.setExposureMode((MMAL_PARAM_EXPOSUREMODE_T) camExposureMode);
        cam.setExposureCompensation(camExposureCompensation);
        cam.setShutterSpeed(camShutterSpeed);
#else
        vector<ofVideoDevice> devices = vidGrabber.listDevices();
        
        for(size_t i = 0; i < devices.size(); i++){
            if(devices[i].bAvailable){
                ofLogNotice() << devices[i].id << ": " << devices[i].deviceName;
            }else{
                ofLogNotice() << devices[i].id << ": " << devices[i].deviceName << " - unavailable ";
            }
        }
        
        videoDevice = settings.getValue("settings:video_device", 0);
        vidGrabber.setDeviceID(videoDevice);
        vidGrabber.setDesiredFrameRate(camFps);
        vidGrabber.initGrabber(camW, camH);
#endif
    }
    
    contourLineWidth = settings.getValue("settings:contour_line_width", 10);
    lineWidth = settings.getValue("settings:line_width", 10);
    alphaVal = settings.getValue("settings:alpha_val", 255);
    decayAlpha = settings.getValue("settings:decay_alpha", 2);
    contourSlices = settings.getValue("settings:contour_slices", 10);
    
    contourAlpha = settings.getValue("settings:contour_alpha", 127);
    pointReadMultiplier = settings.getValue("settings:point_read_multiplier", 1.0);
    translateXorig = settings.getValue("settings:translate_x", 40.0);
    translateYorig = settings.getValue("settings:translate_y", -115.0);
    randomPositionSpread = settings.getValue("settings:random_position_spread", 10.0);
       
    contourMinAreaRadius = settings.getValue("settings:contour_min_radius", 10.0);
    contourMaxAreaRadius = settings.getValue("settings:contour_max_radius", 150.0);
    contourFinder.setMinAreaRadius(contourMinAreaRadius);
    contourFinder.setMaxAreaRadius(contourMaxAreaRadius);
}

void ofApp::randomizePosition() {
    translateX = translateXorig + ofRandom(-randomPositionSpread, randomPositionSpread);
    translateY = translateYorig + ofRandom(-randomPositionSpread, randomPositionSpread);
}

void ofApp::update() {
    bgMeshes.clear();
    
    while (receiver.hasWaitingMessages()) {
        // get the next message
        ofxOscMessage msg;
        receiver.getNextMessage(msg);
        
        if (msg.getAddress() == "/contour") {
            ofMesh bgMesh;
            bgMesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);

            ofBuffer buffer = msg.getArgAsBlob(4);
            vector<glm::vec3> cvPoints = bufferToPoints(buffer);
            
            float widthSmooth = 1;
            float angleSmooth;
            
            int kLimit = (int) cvPoints.size();

            float pointsData[kLimit * 3];

            for (int k=0; k<kLimit; k++) {
                int contourIndex = k * 3;
                
                int me_m_one = k - 1;
                int me_p_one = k + 1;
                if (me_m_one < 0) me_m_one = 0;
                if (me_p_one == kLimit) me_p_one = kLimit - 1;
                
                ofPoint diff = cvPoints[me_p_one] - cvPoints[me_m_one];
                float angle = atan2(diff.y, diff.x);
                
                if (k == 0) {
                    angleSmooth = angle;
                } else {
                    angleSmooth = ofLerpDegrees(angleSmooth, angle, 1.0);
                }
                
                float dist = diff.length();
                
                float w = ofMap(dist, 0, 20, lineWidth, 2, true); //40, 2, true);
                
                widthSmooth = 0.2f * widthSmooth + 0.1f * w;
                
                ofPoint offset;
                offset.x = cos(angleSmooth + PI/2) * widthSmooth;
                offset.y = sin(angleSmooth + PI/2) * widthSmooth;
                
                if (doSpread) {
                    offset.x += ofRandom(-spread, spread);
                    offset.y += ofRandom(-spread, spread);
                }
                
                bgMesh.addVertex(cvPoints[k] + offset);
                bgMesh.addVertex(cvPoints[k] - offset);
            }
                
            bgMeshes.push_back(bgMesh);
        }
    }
    
    if (playLatk) {
        float pos = snd.getPositionMS() / 1000.0;
        //spread += spreadDelta;
        
        if (pos > stopTimesArray[currentFrame]) {
            currentFrame++;
            currentStroke = 0;
            currentPoint = 0;
            spread = spreadOrig;
            randomizePosition();
        }
        
        if (pos > stopTimesArray[int(stopTimesArray.size()) - 1] || currentFrame > int(stopTimesArray.size()) - 1) {
            currentFrame = 0;
            currentStroke = 0;
            currentPoint = 0;
            spread = spreadOrig;
            randomizePosition();
        }
    }

#ifdef TARGET_RASPBERRY_PI
    frame = cam.grab();
#else
    vidGrabber.update();
    if (vidGrabber.isFrameNew()) {
        frame = toCv(vidGrabber.getPixelsRef());
    }
#endif
}

void ofApp::draw() {   
    ofBackground(0);
    fbo.begin();
    ofSetColor(0, decayAlpha);
    ofDrawRectangle(0, 0, fbo.getWidth(), fbo.getHeight());
    
    if (!frame.empty() && !playLatk) {
        int contourCounter = 0;
        
        for (int h=0; h<255; h += int(255/contourSlices)) {
            contourFinder.setThreshold(h);
            contourFinder.findContours(frame);
            
            int n = contourFinder.size();

            for (int h = 0; h < n; h++) {
                ofPolyline line = contourFinder.getPolyline(h);
                vector<glm::vec3> cvPoints = line.getVertices();
                                               
                int cvPointsSize = int(cvPoints.size());
                float pointsData[cvPointsSize * 3];
                
                for (int i = 0; i < cvPointsSize; i++) {
                    int contourIndex = i * 3;
                                      
                    pointsData[contourIndex] = cvPoints[i].x;
                    pointsData[contourIndex+1] = cvPoints[i].y;
                    pointsData[contourIndex+2] = 0.0;
                }
                
                char const * pPoints = reinterpret_cast<char const *>(pointsData);
                std::string pointsString(pPoints, pPoints + sizeof pointsData);
                contourPointsBuffer.set(pointsString);
                
                float colorData[3];
                colorData[0] = 255;
                colorData[1] = 255;
                colorData[2] = 255;
                char const * pColor = reinterpret_cast<char const *>(colorData);
                std::string colorString(pColor, pColor + sizeof colorData);
                contourColorBuffer.set(colorString);
                
                sendOscContours(contourCounter);
                contourCounter++;
            }
        }
    } else if (playLatk) {
        ofPushMatrix();
        ofSetLineWidth(lineWidth);
        ofScale(ofGetWidth() / 128.0, ofGetHeight() / -128.0);
        ofTranslate(translateX, translateY);
        ofNoFill();
        
        for (int j=0; j<currentStroke + 1; j++) {
            fgMesh.clear();
            
            float widthSmooth = 1;
            float angleSmooth;
            
            int kLimit = (int) latk.layers[0].frames[currentFrame].strokes[j].points.size();
            if (j == currentStroke) kLimit = currentPoint + 1;

            float pointsData[kLimit * 3];

            for (int k=0; k<kLimit; k++) {
                int contourIndex = k * 3;
                
                int me_m_one = k - 1;
                int me_p_one = k + 1;
                if (me_m_one < 0) me_m_one = 0;
                if (me_p_one == kLimit) me_p_one = kLimit - 1;
                
                ofPoint diff = latk.layers[0].frames[currentFrame].strokes[j].points[me_p_one] - latk.layers[0].frames[currentFrame].strokes[j].points[me_m_one];
                float angle = atan2(diff.y, diff.x);
                
                if (k == 0) {
                    angleSmooth = angle;
                } else {
                    angleSmooth = ofLerpDegrees(angleSmooth, angle, 1.0);
                }
                
                float dist = diff.length();
                
                float w = ofMap(dist, 0, 20, lineWidth, 2, true); //40, 2, true);
                
                widthSmooth = 0.2f * widthSmooth + 0.1f * w;
                
                ofPoint offset;
                offset.x = cos(angleSmooth + PI/2) * widthSmooth;
                offset.y = sin(angleSmooth + PI/2) * widthSmooth;
                
                if (doSpread) {
                    offset.x += ofRandom(-spread, spread);
                    offset.y += ofRandom(-spread, spread);
                }
                
                fgMesh.addVertex(latk.layers[0].frames[currentFrame].strokes[j].points[k] + offset);
                fgMesh.addVertex(latk.layers[0].frames[currentFrame].strokes[j].points[k] - offset);
                
                if (secondaryOscSend) {
                    pointsData[contourIndex] = latk.layers[0].frames[currentFrame].strokes[j].points[k].x;
                    pointsData[contourIndex+1] = latk.layers[0].frames[currentFrame].strokes[j].points[k].y;
                    pointsData[contourIndex+2] = latk.layers[0].frames[currentFrame].strokes[j].points[k].z;
                }
            }
           
            if (secondaryOscSend) {
                char const * pPoints = reinterpret_cast<char const *>(pointsData);
                std::string pointsString(pPoints, pPoints + sizeof pointsData);
                contourPointsBuffer.set(pointsString);
            }
            
            ofColor col;
            
            if (videoColor) {
                if (ofRandom(1.0) < 0.2) {
                    col = ofColor(13, 197, 255, alphaVal);
                } else {
                    col = ofColor(255, 197, 13, alphaVal);
                }
            } else {
                col = ofColor(255, alphaVal);
            }
            
            ofSetColor(col);
            
            if (secondaryOscSend) {
                float colorData[3];
                colorData[0] = col.r;
                colorData[1] = col.g;
                colorData[2] = col.b;
                char const * pColor = reinterpret_cast<char const *>(colorData);
                std::string colorString(pColor, pColor + sizeof colorData);
                contourColorBuffer.set(colorString);
            }
            
            if (drawWireframe) {
                fgMesh.drawWireframe();
            } else {
                fgMesh.draw();
            }
            
            if (secondaryOscSend) sendOscContours(j);
        }
        ofPopMatrix();

        ofPushMatrix();
        ofScale(ofGetWidth() / camW, ofGetHeight() / camH);

        if (videoColor) {
             if (ofRandom(1.0) < 0.5) {
                 ofSetColor(13, 127, 255, contourAlpha);
             } else {
                 ofSetColor(255, 127, 13, contourAlpha);
             }
         } else {
             ofSetColor(255, contourAlpha);
         }
      
        for (int i=0; i<bgMeshes.size(); i++) {
            if (drawWireframe) {
                bgMeshes[i].drawWireframe();
            } else {
                bgMeshes[i].draw();
            }
        }
        
        ofPopMatrix();
    }
    
    fbo.end();
        
    float width = fbo.getWidth();
    float height = fbo.getHeight();
    
    fbo.getTextureReference().bind();
    shader.begin();
    ofTranslate(width / 2, height / 2);
    ofRotateDeg(fboRotation);
    ofScale(1.0, -1.0, 1.0);
    fbo.draw(-width / 2, -height / 2);
    shader.end();
    fbo.getTextureReference().unbind();
    
    if (playLatk) {
        float pointsSize = latk.layers[0].frames[currentFrame].strokes[currentStroke].points.size() - 1;
        float pointStep_f = ofMap(abs(largestTimeDiff - diffTimesArray[currentFrame]), 0.0, largestTimeDiff, pointsSize, 1.0) * pointReadMultiplier;
        int pointStep = int(pointStep_f);
        if (pointStep < 1) pointStep = 1;
        //cout << pointStep_f << ", " << pointStep << endl;
        
        if (currentPoint < pointsSize) {
            currentPoint += pointStep;
            if (currentPoint > pointsSize) currentPoint = pointsSize;
        }
        
        if (currentStroke < int(latk.layers[0].frames[currentFrame].strokes.size()) - 1 && currentPoint >= int(latk.layers[0].frames[currentFrame].strokes[currentStroke].points.size()) - 1) {
            currentStroke++;
            currentPoint = 0;
        }
    }
    
    spread += spreadDelta;   
}

void ofApp::setupOscSender(ofxOscSender& sender, string& oscSendHost, int oscSendPort) {
    sender.setup(oscSendHost, oscSendPort);
    cout << "\nSending OSC to " << oscSendHost << " on port: " << oscSendPort << endl;
}

void ofApp::setupOscReceiver(ofxOscReceiver& receiver, int oscReceivePort) {
    receiver.setup(oscReceivePort);
    cout << "\nReceiving OSC on port: " << oscReceivePort << endl;
}

vector<glm::vec3> ofApp::bufferToPoints(const ofBuffer& buffer) {
    vector<float> result;
    vector<glm::vec3> points;
    
    if (buffer.size() % sizeof(float) != 0) {
        ofLogError("getFloatsFromBuffer") << "Buffer size is not a multiple of the size of a float.";
    }
    
    const float* floatPtr = reinterpret_cast<const float*>(buffer.getData());
    
    size_t numFloats = buffer.size() / sizeof(float);
    
    result.assign(floatPtr, floatPtr + numFloats);
        
    for (int i=0; i<result.size(); i+= 3) {
        points.push_back(glm::vec3(result[i], result[i+1], result[i+2]));
    }

    return points;
}

void ofApp::sendOscContours(int index) {
    ofxOscMessage msg;
    msg.setAddress("/contour");
    
    msg.addStringArg(hostName);
    msg.addStringArg(sessionId);
    msg.addIntArg(index);
    msg.addBlobArg(contourColorBuffer);
    msg.addBlobArg(contourPointsBuffer);
    msg.addIntArg(timestamp);

    sender.sendMessage(msg);
}

string ofApp::cleanString(string input) {
    ofStringReplace(input, "\n", "");
    ofStringReplace(input, "\r", "");
    return input;
}

// a randomly generated id that isn't saved
string ofApp::getSessionId() {
    string sessionId = "RPi_" + ofGetTimestampString("%y%m%d%H%M%S%i");
    return cleanString(sessionId);
}

// the RPi network hostname
string ofApp::getHostName() {
    string returns;
    ofBuffer hostNameFile;

    try {
#ifdef TARGET_OSX
        returns = cleanString(ofSystem("hostname"));
        returns = ofSplitString(returns, ".")[0];
#endif
#ifdef TARGET_WIN32
        returns = cleanString(ofSystem("hostname"));
        returns = ofSplitString(returns, ".")[0];
#endif
#ifdef TARGET_LINUX
        hostNameFile = ofBufferFromFile("/etc/hostname");
        returns = cleanString(hostNameFile.getText());
#endif
    } catch (std::exception &e) {
        cout << "Exception: " << e.what() << endl;
    }

    if (returns.size() < 1) returns = "unknown";

    cout << "Hostname: " << returns << endl;
    return returns;
}

int ofApp::getTimestamp() {
    int returns = 0;
#if OF_VERSION_MAJOR >= 0 && OF_VERSION_MINOR >= 10
    returns = (int) ofGetSystemTimeMillis();
#else
    returns = (int) ofGetSystemTime();
#endif
    return returns;
}
