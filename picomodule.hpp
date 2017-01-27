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
    std::list<classifier> m_classifiers;

    bool nameExist(std::string);
public:
    PicoModule(boost::shared_ptr<AL::ALBroker>, const std::string&);

    virtual ~PicoModule();

    virtual void init();
    virtual void start();
    virtual void stop();
    virtual void process(AL::ALImage*);

    bool addClassifier(std::string, AL::ALValue, float, float, float, int, int);
    bool removeClassifier(std::string);
    AL::ALValue getClassifierList();
    AL::ALValue getClassifierParameters(std::string);
    bool changeClassifierParameters(std::string, float, float, float, int, int);
    void detectOnImage(AL::ALImage);
};

#endif //PICOMODULE_H
