#include "picomodule.hpp"

#include <opencv2/imgproc/imgproc.hpp>
#include "APIwrappers.h"
extern "C"{
#include "PICO/picornt.h"
}

PicoModule::PicoModule(boost::shared_ptr<AL::ALBroker> broker, const std::string &name) :
    AL::ALModule(broker, name),
    m_memoryProxy(getParentBroker()),
    m_cameraProxy(getParentBroker())
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

void PicoModule::subscribeToVideo()
{
    if(!m_subscriberID.empty()){
        std::cout << "[ERROR][PICO] Module already subscribed to video device!" << std::endl;
        return;
    }
    m_subscriberID = m_cameraProxy.subscribe("PicoModule", AL::k4VGA, AL::kRGBColorSpace, 5);
}
void PicoModule::unsuscribeFromVideo(){
    if(m_subscriberID.empty()){
        std::cout << "[ERROR][PICO] Module is currently not subscribed to video device!" << std::endl;
        return;
    }
    m_cameraProxy.unsubscribe(m_subscriberID);
    m_subscriberID.erase();
}
void PicoModule::captureImage(){
#ifdef MODULE_IS_REMOTE
    AL::ALValue pic = m_cameraProxy.getImageRemote(m_subscriberID);
#else
    AL::ALValue pic = m_cameraProxy.getImageLocal(m_subscriberID);
#endif
    m_img = cv::Mat(cv::Size(pic[0], pic[1]), CV_8UC3);
    m_img.data = (uchar*) pic[6].GetBinary();
    m_cameraProxy.releaseImage(m_subscriberID);
}

void *PicoModule::serviceFunction(void *ptr)
{
    PicoModule* _this = (PicoModule *)ptr;
    while(_this->m_serviceLoop){
        uint8_t* pixels;
        int nrows, ncols, ldim;
        cv::Mat imgGray;

        #define MAXNDETECTIONS 2048
        int ndetections;
        float rcsq[4*MAXNDETECTIONS];

        if(_this->m_img.channels() != 1) cv::cvtColor(_this->m_img, imgGray, CV_RGB2GRAY);
        else imgGray = _this->m_img;

        _this->captureImage();

        pixels = (uint8_t*)imgGray.data;
        nrows = imgGray.rows;
        ncols = imgGray.cols;
        ldim = imgGray.step;

        ndetections = find_objects(rcsq, MAXNDETECTIONS, _this->m_cascade, _this->m_angle, pixels, nrows, ncols, ldim, _this->m_scalefactor, _this->m_stridefactor, _this->m_minsize, MIN(nrows, ncols));

        for(int k = 0; k<ndetections; k++){
            std::vector<float> tmp;
            tmp.push_back(rcsq[4*k + 0]);
            tmp.push_back(rcsq[4*k + 1]);
            tmp.push_back(rcsq[4*k + 2]);
            tmp.push_back(rcsq[4*k + 3]);
            _this->m_memoryProxy.raiseEvent("picoDetections", tmp);
        }
    }
}

void PicoModule::startService(std::string cascade, float angle = 0, float scalefactor = 1.1, float stridefactor = 0.1, int minsize = 128){
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


    subscribeToVideo();
    m_serviceLoop = true;
    int tmp = pthread_create(&m_serviceThread, NULL, serviceFunction, this);
    if (tmp) {
        std::cout << "[ERROR][PICO] Service thread failed to initialize" << std::endl;
        m_serviceLoop = false;
        unsuscribeFromVideo();
    }
}
void PicoModule::stopService(){
    if(m_serviceLoop){
        m_serviceLoop = false;
        pthread_join(m_serviceThread, NULL);
        unsuscribeFromVideo();
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

    m_img = ALtoMAT(image);

    uint8_t* pixels;
    int nrows, ncols, ldim;
    cv::Mat imgGray;

    #define MAXNDETECTIONS 2048
    int ndetections;
    float rcsq[4*MAXNDETECTIONS];

    if(m_img.channels() != 1) cv::cvtColor(m_img, imgGray, CV_RGB2GRAY);
    else imgGray = m_img;

    captureImage();

    pixels = (uint8_t*)imgGray.data;
    nrows = imgGray.rows;
    ncols = imgGray.cols;
    ldim = imgGray.step;

    ndetections = find_objects(rcsq, MAXNDETECTIONS, m_cascade, m_angle, pixels, nrows, ncols, ldim, m_scalefactor, m_stridefactor, m_minsize, MIN(nrows, ncols));

    for(int k = 0; k<ndetections; k++){
        std::vector<float> tmp;
        tmp.push_back(rcsq[4*k + 0]);
        tmp.push_back(rcsq[4*k + 1]);
        tmp.push_back(rcsq[4*k + 2]);
        tmp.push_back(rcsq[4*k + 3]);
        m_memoryProxy.raiseEvent("picoDetections", tmp);
    }

}
