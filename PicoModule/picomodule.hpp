#ifndef PICOMODULE_H
#define PICOMODULE_H

#include <iostream>
#include <pthread.h>

#include <alcommon/albroker.h>
#include <alproxies/alvideodeviceproxy.h>
#include <alproxies/almemoryproxy.h>
#include <alvision/alvisiondefinitions.h>

#include <opencv2/core/core.hpp>

class PicoModule : public AL::ALModule
{
private:
    AL::ALVideoDeviceProxy cameraProxy;
    AL::ALMemoryProxy memoryProxy;
    cv::Mat img;
    pthread_t serviceThread;
    volatile bool serviceLoop;

    std::string subscriberID;

    int minsize;
    float angle;
    float scalefactor;
    float stridefactor;
    void* cascade;

    void subscribeToVideo();
    void unsuscribeFromVideo();

protected:
    void captureImage();
    static void *serviceFunction(void *ptr);

public:
    PicoModule(boost::shared_ptr<AL::ALBroker>, const std::string&);

    virtual ~PicoModule();

    virtual void init();

    void startService(std::string, float, float, float, int);
    void stopService();

    void detectOnImage(AL::ALValue, std::string, float, float, float, int);
};

#endif //PICOMODULE_H
