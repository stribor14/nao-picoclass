#include "picomodule.hpp"

#include <opencv2/imgproc/imgproc.hpp>
#include "PICO/picornt.h"
#include "APIwrappers.h"

PicoModule::PicoModule(boost::shared_ptr<AL::ALBroker> broker, const std::string &name) : AL::ALModule(broker, name), memoryProxy(getParentBroker()){
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
    cameraProxy = AL::ALVideoDeviceProxy("nao.local");
    memoryProxy.declareEvent("picoDetections", "PicoModule");
}

void PicoModule::subscribeToVideo()
{
    if(!subscriberID.empty()){
        std::cout << "[ERROR][PICO] Module already subscribed to video device!" << std::endl;
        return;
    }
    subscriberID = cameraProxy.subscribe("PicoModule", AL::k4VGA, AL::kRGBColorSpace, 5);
}
void PicoModule::unsuscribeFromVideo(){
    if(subscriberID.empty()){
        std::cout << "[ERROR][PICO] Module is currently not subscribed to video device!" << std::endl;
        return;
    }
    cameraProxy.unsubscribe(subscriberID);
    subscriberID.erase();
}
void PicoModule::captureImage(){
#ifdef MODULE_IS_REMOTE
    AL::ALValue pic = cameraProxy.getImageRemote(subscriberID);
#else
    AL::ALValue pic = cameraProxy.getImageLocal(subscriberID);
#endif
    img = cv::Mat(cv::Size(pic[0], pic[1]), CV_8UC3);
    img.data = (uchar*) pic[6].GetBinary();
    cameraProxy.releaseImage(subscriberID);
}

void *PicoModule::serviceFunction(void *ptr)
{
    PicoModule* _this = (PicoModule *)ptr;
    while(_this->serviceLoop){
        uint8_t* pixels;
        int nrows, ncols, ldim;
        cv::Mat imgGray;

        #define MAXNDETECTIONS 2048
        int ndetections;
        float rcsq[4*MAXNDETECTIONS];

        if(_this->img.channels() != 1) cv::cvtColor(_this->img, imgGray, CV_RGB2GRAY);
        else imgGray = _this->img;

        _this->captureImage();

        pixels = (uint8_t*)imgGray.data;
        nrows = imgGray.rows;
        ncols = imgGray.cols;
        ldim = imgGray.step;

        ndetections = find_objects(rcsq, MAXNDETECTIONS, _this->cascade, _this->angle, pixels, nrows, ncols, ldim, _this->scalefactor, _this->stridefactor, _this->minsize, MIN(nrows, ncols));

        for(int k = 0; k<ndetections; k++){
            std::vector<float> tmp;
            tmp.push_back(rcsq[4*k + 0]);
            tmp.push_back(rcsq[4*k + 1]);
            tmp.push_back(rcsq[4*k + 2]);
            tmp.push_back(rcsq[4*k + 3]);
            _this->memoryProxy.raiseEvent("picoDetections", tmp);
        }
    }
}

void PicoModule::startService(std::string cascade, float angle = 0, float scalefactor = 1.1, float stridefactor = 0.1, int minsize = 128){
    this->angle = angle == -1 ? 0 : angle*2*3.1415926;
    this->angle = scalefactor == -1 ? 1.1 : scalefactor;
    this->angle = stridefactor == -1 ? 0.1: stridefactor;
    this->angle = minsize == -1 ? 128: minsize;

    FILE* file = fopen(cascade.c_str(), "rb");
    if(!file){
        std::cout << "[ERROR][PICO] Cannot read cascade from: " << cascade << std::endl;
        return;
    }
    fseek(file, 0L, SEEK_END);
    int size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    this->cascade = malloc(size);
    if(!this->cascade || size!=fread(this->cascade, 1, size, file)){
        std::cout << "[ERROR][PICO] Failure while reading cascade from: " << cascade << std::endl;
        free(this->cascade);
        return;
    }
    fclose(file);


    subscribeToVideo();
    serviceLoop = true;
    int tmp = pthread_create(&serviceThread, NULL, serviceFunction, this);
    if (tmp) {
        std::cout << "[ERROR][PICO] Service thread failed to initialize" << std::endl;
        serviceLoop = false;
        unsuscribeFromVideo();
    }
}
void PicoModule::stopService(){
    if(serviceLoop){
        serviceLoop = false;
        pthread_join(serviceThread, NULL);
        unsuscribeFromVideo();
        free(this->cascade);
    } else {
        std::cout << "[ERROR][PICO] Service not started" << std::endl;
    }
}

void PicoModule::detectOnImage(AL::ALValue image, std::string cascade, float angle = 0, float scalefactor = 1.1, float stridefactor = 0.1, int minsize = 128){
    this->angle = angle == -1 ? 0 : angle*2*3.1415926;
    this->angle = scalefactor == -1 ? 1.1 : scalefactor;
    this->angle = stridefactor == -1 ? 0.1: stridefactor;
    this->angle = minsize == -1 ? 128: minsize;
    FILE* file = fopen(cascade.c_str(), "rb");
    if(!file){
        std::cout << "[ERROR][PICO] Cannot read cascade from: " << cascade << std::endl;
        return;
    }
    fseek(file, 0L, SEEK_END);
    int size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    this->cascade = malloc(size);
    if(!this->cascade || size!=fread(this->cascade, 1, size, file)){
        std::cout << "[ERROR][PICO] Failure while reading cascade from: " << cascade << std::endl;
        free(this->cascade);
        return;
    }
    fclose(file);

    this->img = ALtoMAT(image);

    uint8_t* pixels;
    int nrows, ncols, ldim;
    cv::Mat imgGray;

    #define MAXNDETECTIONS 2048
    int ndetections;
    float rcsq[4*MAXNDETECTIONS];

    if(this->img.channels() != 1) cv::cvtColor(this->img, imgGray, CV_RGB2GRAY);
    else imgGray = this->img;

    this->captureImage();

    pixels = (uint8_t*)imgGray.data;
    nrows = imgGray.rows;
    ncols = imgGray.cols;
    ldim = imgGray.step;

    ndetections = find_objects(rcsq, MAXNDETECTIONS, this->cascade, this->angle, pixels, nrows, ncols, ldim, this->scalefactor, this->stridefactor, this->minsize, MIN(nrows, ncols));

    for(int k = 0; k<ndetections; k++){
        std::vector<float> tmp;
        tmp.push_back(rcsq[4*k + 0]);
        tmp.push_back(rcsq[4*k + 1]);
        tmp.push_back(rcsq[4*k + 2]);
        tmp.push_back(rcsq[4*k + 3]);
        this->memoryProxy.raiseEvent("picoDetections", tmp);
    }

}
