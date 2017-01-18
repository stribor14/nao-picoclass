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


PicoModule::PicoModule(boost::shared_ptr<AL::ALBroker> broker, const std::string &name) :
    AL::ALVisionExtractor(broker, name, AL::k4VGA, AL::k4VGA, AL::kRGBColorSpace, 5),
    m_memoryProxy(getParentBroker())
{
    setModuleDescription("Image classification module based on Pico algorithm");

    functionName("setParameters", getName(), "Set parameters for classification");
    BIND_METHOD(PicoModule::setParameters);
    addParam("cascade", "Path to cascade file [string]");
    addParam("angle", "Cascade rotation [float = 0][rad]");
    addParam("scalefactor", "Scale factor during multiscale detection [float = 1.1][\%]");
    addParam("stridefactor", "Strade factor for window movement between neighboring detections [float = 0.1][\%]");
    addParam("minsize", "Minimum size for detected object [int = 128][pix]");
    addParam("treshold", "Detection quality treshold");

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
    m_serviceLoop = false;

    m_angle = 0;
    m_scalefactor = 1.1;
    m_stridefactor = 0.1;
    m_minsize = 128;
    m_treshold = 5.0;

    readCascade("/home/nao/facefinder");
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
    int ndetections;
    float rcsq[4*maxDetections];

    cv::Mat imgGray;
    cv::Mat imgOrig = AL::aLImageToCvMat(*img);

    if(imgOrig.channels() != 1) cv::cvtColor(imgOrig, imgGray, CV_RGB2GRAY);
    else imgGray = imgOrig;

    uint8_t* pixels = (uint8_t*)imgGray.data;
    int nrows = imgGray.rows;
    int ncols = imgGray.cols;
    int ldim = imgGray.step;

    ndetections = find_objects(rcsq, maxDetections, m_cascade, m_angle, pixels, nrows, ncols, ldim, m_scalefactor, m_stridefactor, m_minsize, MIN(nrows, ncols));

    for(int k = 0; k<ndetections; k++){
        if(rcsq[4*k + 3] < m_treshold) continue;
        std::vector<float> tmp;
        tmp.push_back(rcsq[4*k + 0]);
        tmp.push_back(rcsq[4*k + 1]);
        tmp.push_back(rcsq[4*k + 2]);
        tmp.push_back(rcsq[4*k + 3]);
        m_memoryProxy.raiseEvent("picoDetections", tmp);
        #ifdef MODULE_IS_REMOTE
            cv::circle(imgOrig, cv::Point(static_cast<int>(rcsq[4*k + 1]),static_cast<int>(rcsq[4*k + 0])), static_cast<int>(rcsq[4*k + 2]/2), cv::Scalar( 0, 0, 255 ), 5);
        #endif
    }
    #ifdef MODULE_IS_REMOTE
        if(ndetections)
            cv::imwrite("slika.png",imgOrig);
    #endif
}

void PicoModule::readCascade(std::string cascade){
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
}

void PicoModule::setParameters(std::string cascade, float angle, float scalefactor, float stridefactor, int minsize, int treshold){
    m_angle = angle < 0 ? 0 : angle*2*3.1415926;
    m_scalefactor = scalefactor < 0 ? 1.1 : scalefactor;
    m_stridefactor = stridefactor < 0 ? 0.1 : stridefactor;
    m_minsize = minsize < 0 ? 128 : minsize;
    m_treshold = treshold < 0 ? 10.0 : treshold;

    readCascade(cascade);
}

void PicoModule::detectOnImage(AL::ALValue image){
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
        if(rcsq[4*k + 3] < m_treshold) continue;
        std::vector<float> tmp;
        tmp.push_back(rcsq[4*k + 0]);
        tmp.push_back(rcsq[4*k + 1]);
        tmp.push_back(rcsq[4*k + 2]);
        tmp.push_back(rcsq[4*k + 3]);
        m_memoryProxy.raiseEvent("picoDetections", tmp);
        #ifdef MODULE_IS_REMOTE
            cv::circle(imgOrig, cv::Point(static_cast<int>(rcsq[4*k + 1]),static_cast<int>(rcsq[4*k + 0])), static_cast<int>(rcsq[4*k + 2]/2), cv::Scalar( 0, 0, 255 ), 5);
        #endif
    }
    #ifdef MODULE_IS_REMOTE
        if(ndetections)
            cv::imwrite("slika.png",imgOrig);
    #endif
}
