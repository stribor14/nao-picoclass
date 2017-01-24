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

## Module event:  "*picoDetections*"
    Image timestamp:
      eventValue[0]  - seconds
      eventValue[1]  - microseconds
    Image size:
      eventValue[2]  - width
      eventValue[3]  - height
    Detected objects:
      eventValue[4+][0] - classifier name
      eventValue[4+][0] - X coordinate
      eventValue[4+][0] - Y coordinate
      eventValue[4+][0] - Diameter
      eventValue[4+][0] - Certainty
