#ifndef PICOMODULE_H
#define PICOMODULE_H

#include <iostream>
#include <pthread.h>

#include <alcommon/albroker.h>
#include <alproxies/alvideodeviceproxy.h>
#include <alproxies/almemoryproxy.h>
#include <alvision/alvisiondefinitions.h>
#include <alvision/alvisionextractor.h>

#include <opencv2/core/core.hpp>

class PicoModule : public AL::ALVisionExtractor
{
private:
    AL::ALMemoryProxy m_memoryProxy;
    pthread_t m_serviceThread;
    volatile bool m_serviceLoop;

    std::string m_subscriberID;

    int m_minsize;
    float m_angle;
    float m_scalefactor;
    float m_stridefactor;
    void* m_cascade;

    void subscribeToVideo();
    void unsuscribeFromVideo();

protected:
    void captureImage();
    static void *serviceFunction(void *ptr);

public:
    PicoModule(boost::shared_ptr<AL::ALBroker>, const std::string&);

    virtual ~PicoModule();

    virtual void init();
    virtual void start();
    virtual void stop();
    virtual void process(AL::ALImage *img);

    void startService(std::string, float, float, float, int);
    void stopService();

    void detectOnImage(AL::ALValue, std::string, float, float, float, int);
};

#endif //PICOMODULE_H
