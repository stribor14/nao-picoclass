# PicoClass

Naoqi(2.1) module for visual classification based on PICO classifying algorithm (https://github.com/nenadmarkus/pico)

## Module methods:
    void addClassifier(className, cascade, angle, scalefactor, stridefactor, minsize, treshold)   
    void removeClassifier(className)  
    AL::ALValue getClassifierList()  
    AL::ALValue getClassifierParameters(className)  
    void detectOnImage(image)  
    void subscribe(subscriberName) 
    void unsubscribe(subscriberName)
    void setColorSpace(colorSpace)  
    void setFrameRate(frameRate)  
    void setResolution(resolution)  
    void setActiveCamera(cameraID)  
    int getColorSpace()  
    int getFrameRate() 
    int getResolution() 
    int getActiveCamera()
    void pause(bool)
    bool isPaused()
    bool isProcessing()
    

## Module event:  "*picoDetections*"
    Image timestamp:
      eventValue[0]  - seconds
      eventValue[1]  - microseconds
    Image size:
      eventValue[2]  - width
      eventValue[3]  - height
    Detected objects:
      eventValue[4+][0] - classifier name
      eventValue[4+][1] - X coordinate
      eventValue[4+][2] - Y coordinate
      eventValue[4+][3] - Diameter
      eventValue[4+][4] - Certainty

## Example:
    broker = AL::ALBroker("PicoModule", "nao.local")
    classNames = broker.call("getClassifierList")
    if(!ourClassLoaded(classNames))
        broker.call("addClassifier", "face", "/home/nao/facefinder", -1, -1, -1, -1, -1)
        # if negative parameter value, module uses default one
    broker.call("setCamera", 0)
    broker.call("subscribe", "myName") # process starts if there is classifier
    if(broker.call("isProcessing")
        # TODO: add ALMemoryProxy and listen do "PicoModule" event "picoDetections"
