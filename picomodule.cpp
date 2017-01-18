#include "picomodule.hpp"

#include <opencv2/imgproc/imgproc.hpp>
#include "APIwrappers.h"
extern "C"{
#include "PICO/picornt.h"
}

void PicoModule::start()
{

}

void PicoModule::stop()
{

}

PicoModule::PicoModule(boost::shared_ptr<AL::ALBroker> broker, const std::string &name) :
    AL::ALVisionExtractor(broker, name, AL::k4VGA, AL::k4VGA, AL::kYuvColorSpace, 5),
    m_memoryProxy(getParentBroker())
{
    setModuleDescription("Image classification module based on Pico algorithm");

    functionName("startService", getName(), "Starts realtime classification from NAO camera.");
    BIND_METHOD(PicoModule::startService);
    addParam("cascade", "Path to cascade file [string]");
    addParam("angle", "Cascade rotation [float = 0][rad]");
    addParam("scalefactor", "Scale factor during multiscale detection [float = 1.1][\%]");
    addParam("stridefactor", "Strade factor for window movement between neighboring detections [float = 0.1][\%]");
    addParam("minsize", "Minimum size for detected object [int = 128][pix]");


    functionName("stopService", getName(), "Stops the service.");
    BIND_METHOD(PicoModule::startService);

    functionName("detectOnImage", getName(), "Run detection on one image.");
    BIND_METHOD(PicoModule::detectOnImage);
    addParam("image", "Input image [ALValue]");
    addParam("cascade", "Path to cascade file [string]");
    addParam("angle", "Cascade rotation [float = 0][rad]");
    addParam("scalefactor", "Scale factor during multiscale detection [float = 1.1][\%]");
    addParam("stridefactor", "Strade factor for window movement between neighboring detections [float = 0.1][\%]");
    addParam("minsize", "Minimum size for detected object [int = 128][pix]");

}

PicoModule::~PicoModule(){

}

void PicoModule::init(){
    m_memoryProxy.declareEvent("picoDetections", "PicoModule");
}

void PicoModule::process(AL::ALImage *img)
{
    if(m_serviceLoop){
        int maxDetections = 2048;
        int ndetections;
        float rcsq[4*maxDetections];

        uint8_t* pixels = (uint8_t*)img->getData();
        int nrows = img->getHeight();
        int ncols = img->getWidth();
        int& ldim = nrows;

        ndetections = find_objects(rcsq, maxDetections, m_cascade, m_angle, pixels, nrows, ncols, ldim, m_scalefactor, m_stridefactor, m_minsize, MIN(nrows, ncols));

        for(int k = 0; k<ndetections; k++){
            std::vector<float> tmp;
            tmp.push_back(rcsq[4*k + 0]);
            tmp.push_back(rcsq[4*k + 1]);
            tmp.push_back(rcsq[4*k + 2]);
            tmp.push_back(rcsq[4*k + 3]);
            m_memoryProxy.raiseEvent("picoDetections", tmp);
        }
    }
}


void PicoModule::startService(std::string cascade, float angle = 0, float scalefactor = 1.1, float stridefactor = 0.1, int minsize = 128){
    if(m_serviceLoop){
        std::cout << "[ERROR][PICO] Service is already started" << std::endl;
        return;
    }
    m_angle = angle == -1 ? 0 : angle*2*3.1415926;
    m_angle = scalefactor == -1 ? 1.1 : scalefactor;
    m_angle = stridefactor == -1 ? 0.1: stridefactor;
    m_angle = minsize == -1 ? 128: minsize;

    FILE* file = fopen(cascade.c_str(), "rb");
    if(!file){
        std::cout << "[ERROR][PICO] Cannot read cascade from: " << cascade << std::endl;
        return;
    }
    fseek(file, 0L, SEEK_END);
    int size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    m_cascade = malloc(size);
    if(!m_cascade || size!=fread(m_cascade, 1, size, file)){
        std::cout << "[ERROR][PICO] Failure while reading cascade from: " << cascade << std::endl;
        free(m_cascade);
        return;
    }
    fclose(file);
    m_serviceLoop = true;
}
void PicoModule::stopService(){
    if(m_serviceLoop){
        m_serviceLoop = false;
        free(m_cascade);
    } else {
        std::cout << "[ERROR][PICO] Service not started" << std::endl;
    }
}

void PicoModule::detectOnImage(AL::ALValue image, std::string cascade, float angle = 0, float scalefactor = 1.1, float stridefactor = 0.1, int minsize = 128){
    m_angle = angle == -1 ? 0 : angle*2*3.1415926;
    m_angle = scalefactor == -1 ? 1.1 : scalefactor;
    m_angle = stridefactor == -1 ? 0.1: stridefactor;
    m_angle = minsize == -1 ? 128: minsize;
    FILE* file = fopen(cascade.c_str(), "rb");
    if(!file){
        std::cout << "[ERROR][PICO] Cannot read cascade from: " << cascade << std::endl;
        return;
    }
    fseek(file, 0L, SEEK_END);
    int size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    m_cascade = malloc(size);
    if(!m_cascade || size!=fread(m_cascade, 1, size, file)){
        std::cout << "[ERROR][PICO] Failure while reading cascade from: " << cascade << std::endl;
        free(m_cascade);
        return;
    }
    fclose(file);

    cv::Mat imgOrig = ALtoMAT(image);
    cv::Mat imgGray;

    int maxDetections = 2048;
    float rcsq[4*maxDetections];

    if(imgOrig.channels() != 1) cv::cvtColor(imgOrig, imgGray, CV_RGB2GRAY);
    else imgGray = imgOrig;

    uint8_t* pixels = (uint8_t*)imgGray.data;
    int nrows = imgGray.rows;
    int ncols = imgGray.cols;
    int ldim = imgGray.step;

    int ndetections = find_objects(rcsq, maxDetections, m_cascade, m_angle, pixels, nrows, ncols, ldim, m_scalefactor, m_stridefactor, m_minsize, MIN(nrows, ncols));

    for(int k = 0; k<ndetections; k++){
        std::vector<float> tmp;
        tmp.push_back(rcsq[4*k + 0]);
        tmp.push_back(rcsq[4*k + 1]);
        tmp.push_back(rcsq[4*k + 2]);
        tmp.push_back(rcsq[4*k + 3]);
        m_memoryProxy.raiseEvent("picoDetections", tmp);
    }

}
