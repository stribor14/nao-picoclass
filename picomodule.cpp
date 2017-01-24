#include "picomodule.hpp"

#include <alvision/alimage_opencv.h>
#include <opencv2/imgproc/imgproc.hpp>
#include "APIwrappers.h"
extern "C"{
#include "PICO/picornt.h"
}

#ifdef MODULE_IS_REMOTE
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#endif

// declare default classifier parameters
#define def_angle 0
#define def_scalefactor 1.1
#define def_stridefactor 0.1
#define def_minsize 128
#define def_treshold  5.0

bool PicoModule::nameExist(std::string name)
{
    for(std::list<classifier>::iterator it = m_classifiers.begin(); it != m_classifiers.end(); ++it)
        if(!name.compare(it->name)) return true;
    return false;
}

PicoModule::PicoModule(boost::shared_ptr<AL::ALBroker> broker, const std::string &name) :
    AL::ALVisionExtractor(broker, name, AL::kVGA, AL::kYuvColorSpace, 5),
    m_memoryProxy(getParentBroker())
{
    setModuleDescription("Image classification module based on Pico algorithm");

    functionName("addClassifier", getName(), "Add classifier and parameters for classification (pass negative value to set default one)");
    BIND_METHOD(PicoModule::addClassifier);
    addParam("name", "Name of classifier [string]");
    addParam("cascade", "Binary data or path to cascade file [ALValue(binary)] [ALValue(string)]");
    addParam("angle", "Cascade rotation [float = 0][rad]");
    addParam("scalefactor", "Scale factor during multiscale detection [float = 1.1][\%]");
    addParam("stridefactor", "Strade factor for window movement between neighboring detections [float = 0.1][\%]");
    addParam("minsize", "Minimum size for detected object [int = 128][pix]");
    addParam("treshold", "Detection quality treshold [float = 5.0]");

    functionName("removeClassifier", getName(), "Remove classifier by name");
    BIND_METHOD(PicoModule::removeClassifier);
    addParam("name", "Name of classifier [string]");

    functionName("getClassifierList", getName(), "Get name list of loaded classifiers");
    BIND_METHOD(PicoModule::getClassifierList);
    setReturn("nameList", "Array of classifier names [ALValue]");

    functionName("getClassifierParameters", getName(), "Get list of classifier parrameters");
    BIND_METHOD(PicoModule::getClassifierParameters);
    addParam("name", "Name of classifier [string]");
    setReturn("parameters", "Array of classifier parameters (FALSE if name doesn\'t exist) [ALValue]");

    functionName("detectOnImage", getName(), "Run detection on one image (pass negative value to set default one)");
    BIND_METHOD(PicoModule::detectOnImage);
    addParam("image", "Input image [ALImage]");
}

PicoModule::~PicoModule(){
    for(std::list<classifier>::iterator it = m_classifiers.begin(); it != m_classifiers.end(); ++it)
        free(it->cascade);
}

void PicoModule::init(){
    m_memoryProxy.declareEvent("picoDetections", "PicoModule");
}

void PicoModule::start()
{

}

void PicoModule::stop()
{

}

void PicoModule::process(AL::ALImage *img)
{
    int maxDetections = 2048;

    cv::Mat imgGray;
    cv::Mat imgOrig = AL::aLImageToCvMat(*img);

    if(imgOrig.channels() != 1) cv::cvtColor(imgOrig, imgGray, CV_RGB2GRAY);
    else imgGray = imgOrig;

    uint8_t* pixels = (uint8_t*)imgGray.data;
    int nrows = imgGray.rows;
    int ncols = imgGray.cols;
    int ldim = imgGray.step;

    bool raise = false;
    AL::ALValue out;
    //push image timestamp (secs, msecs)
    out.arrayPush((int)(img->getTimeStamp()/1000000));
    out.arrayPush((int)(img->getTimeStamp()%1000000));
    //push image size (width, height)
    out.arrayPush(img->getWidth());
    out.arrayPush(img->getHeight());

    for(std::list<classifier>::iterator it = m_classifiers.begin(); it != m_classifiers.end(); ++it){
        float rcsq[4*maxDetections];
        int ndetections = find_objects(rcsq, maxDetections, it->cascade, it->angle, pixels, nrows, ncols, ldim, it->scalefactor, it->stridefactor, it->minsize, MIN(nrows, ncols));

        if(ndetections){
            for(int k = 0; k<ndetections; k++){
                if(rcsq[4*k + 3] < it->treshold) continue;
                raise = true;
                AL::ALValue tmp;
                tmp.arrayPush(it->name); // classifier name
                tmp.arrayPush(rcsq[4*k + 1]); // center_x
                tmp.arrayPush(rcsq[4*k + 0]); // center_y
                tmp.arrayPush(rcsq[4*k + 2]); // size
                tmp.arrayPush(rcsq[4*k + 3]); // certainty
                out.arrayPush(tmp);
            }
        }
    }
    if(raise) m_memoryProxy.raiseEvent("picoDetections", out);
}

void PicoModule::addClassifier(std::string name, AL::ALValue cascade, float angle, float scalefactor, float stridefactor, int minsize, int treshold){
    if(nameExist(name)){
        std::cout << "[ERROR][PICO] Classifier with name \"" << name << "\" already exist." << std::endl;
        return;
    }
    classifier temp;
    temp.name = name;
    temp.angle = angle < 0 ? def_angle : angle*2*3.1415926;
    temp.scalefactor = scalefactor < 0 ? def_scalefactor : scalefactor;
    temp.stridefactor = stridefactor < 0 ? def_stridefactor : stridefactor;
    temp.minsize = minsize < 0 ? def_minsize : minsize;
    temp.treshold = treshold < 0 ? def_treshold : treshold;

    if(cascade.isString()){
        std::string path = cascade.toString();
        if(path.at(0) == '\"') path = path.substr(1, path.size()-2);
        FILE* file = fopen(path.c_str(), "rb");
        if(!file){
            std::cout << "[ERROR][PICO] Cannot read cascade from: " << cascade.toString().c_str() << std::endl;
            return;
        }
        fseek(file, 0L, SEEK_END);
        int size = ftell(file);
        fseek(file, 0L, SEEK_SET);
        temp.cascade = malloc(size);
        if(!temp.cascade || size!=fread(temp.cascade, 1, size, file)){
            std::cout << "[ERROR][PICO] Failure while reading cascade from: " << cascade.toString() << std::endl;
            free(temp.cascade);
        } else m_classifiers.push_back(temp);
        fclose(file);
    } else if(cascade.isBinary()){
        int size = cascade.getSize();
        temp.cascade = malloc(size);
        memcpy(temp.cascade, cascade.GetBinary(), size);
    } else {
        std::cout << "[ERROR][PICO] Cascade is nor data[Binary] nor path[String]"<< std::endl;
    }
}

void PicoModule::removeClassifier(std::string name){
    if(!nameExist(name)){
        std::cout << "[ERROR][PICO] Classifier with name \"" << name << "\" doesn\'t exist." << std::endl;
        return;
    }
    for(std::list<classifier>::iterator it = m_classifiers.begin(); it != m_classifiers.end(); ++it)
        if(!name.compare(it->name)){
            free(it->cascade);
            m_classifiers.erase(it);
            return;
        }
}

AL::ALValue PicoModule::getClassifierList()
{
    AL::ALValue out;
    for(std::list<classifier>::iterator it = m_classifiers.begin(); it != m_classifiers.end();++it)
        out.arrayPush(it->name);
    return out;
}

AL::ALValue PicoModule::getClassifierParameters(std::string name)
{
    if(!nameExist(name)){
        std::cout << "[ERROR][PICO] Classifier with name \"" << name << "\" doesn\'t exist." << std::endl;
        return AL::ALValue(false);
    }
    for(std::list<classifier>::iterator it = m_classifiers.begin(); it != m_classifiers.end(); ++it)
        if(!name.compare(it->name)){
            AL::ALValue tmp;
            tmp.arrayPush(it->name);
            tmp.arrayPush(it->angle);
            tmp.arrayPush(it->scalefactor);
            tmp.arrayPush(it->stridefactor);
            tmp.arrayPush(it->minsize);
            tmp.arrayPush(it->treshold);
            return tmp;
        }
    return AL::ALValue(false);
}

void PicoModule::detectOnImage(AL::ALImage image){
    process(&image);
}
