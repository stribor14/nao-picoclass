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

    int m_minsize;
    float m_angle;
    float m_scalefactor;
    float m_stridefactor;
    float m_treshold;
    void* m_cascade;

    void readCascade(std::string cascade);

public:
    PicoModule(boost::shared_ptr<AL::ALBroker>, const std::string&);

    virtual ~PicoModule();

    virtual void init();
    virtual void start();
    virtual void stop();
    virtual void process(AL::ALImage *img);

    void setParameters(std::string, float, float, float, int, int treshold);
    void detectOnImage(AL::ALValue);
};

#endif //PICOMODULE_H
