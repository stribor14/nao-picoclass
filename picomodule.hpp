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

struct classifier{
    std::string name;
    int minsize;
    float angle;
    float scalefactor;
    float stridefactor;
    float treshold;
    void* cascade;
};

class PicoModule : public AL::ALVisionExtractor
{
private:
    AL::ALMemoryProxy m_memoryProxy;

    int def_minsize;
    float def_angle;
    float def_scalefactor;
    float def_stridefactor;
    float def_treshold;
    std::list<classifier> m_classifiers;

public:
    PicoModule(boost::shared_ptr<AL::ALBroker>, const std::string&);

    virtual ~PicoModule();

    virtual void subscribe(const std::string&, const int&, const float&);
    void subscribe(const std::string&);

    virtual void init();
    virtual void start();
    virtual void stop();
    virtual void process(AL::ALImage*);

    void addClassifier(std::string, std::string, float, float, float, int, int);
    void removeClassifier(std::string);

    void detectOnImage(AL::ALImage);
};

#endif //PICOMODULE_H
