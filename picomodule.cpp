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
    AL::ALVisionExtractor(broker, name, AL::k4VGA, AL::kRGBColorSpace, 5),
    m_memoryProxy(getParentBroker())
{
    setModuleDescription("Image classification module based on Pico algorithm");

    functionName("addClassifier", getName(), "Add classifier and parameters for classification (pass negative value to set default one)");
    BIND_METHOD(PicoModule::addClassifier);
    addParam("name", "Name of classifier [string]");
    addParam("cascade", "Path to cascade file [string]");
    addParam("angle", "Cascade rotation [float = 0][rad]");
    addParam("scalefactor", "Scale factor during multiscale detection [float = 1.1][\%]");
    addParam("stridefactor", "Strade factor for window movement between neighboring detections [float = 0.1][\%]");
    addParam("minsize", "Minimum size for detected object [int = 128][pix]");
    addParam("treshold", "Detection quality treshold [float = 5.0]");

    functionName("removeClassifier", getName(), "Remove classifier by name");
    BIND_METHOD(PicoModule::removeClassifier);
    addParam("name", "Name of classifier [string]");

    functionName("detectOnImage", getName(), "Run detection on one image (pass negative value to set default one)");
    BIND_METHOD(PicoModule::detectOnImage);
    addParam("image", "Input image [ALImage]");
}

PicoModule::~PicoModule(){
    for(std::list<classifier>::iterator it = m_classifiers.begin(); it != m_classifiers.end(); ++it)
        free(it->cascade);
}

void PicoModule::subscribe(const std::string &pSubscribedName, const int &pPeriod, const float &pPrecision)
{
    if(m_classifiers.size()) AL::ALVisionExtractor::subscribe(pSubscribedName, pPeriod, pPrecision);
    else std::cout << "[ERROR][PICO] Subscription refused: no classifiers set." << std::endl;
}

void PicoModule::subscribe(const std::string &pSubscribedName)
{
    if(m_classifiers.size()) AL::ALVisionExtractor::subscribe(pSubscribedName);
    else std::cout << "[ERROR][PICO] Subscription refused: no classifiers set." << std::endl;
}

void PicoModule::init(){
    m_memoryProxy.declareEvent("picoDetections", "PicoModule");

    // declare default classifier parameters
    def_angle = 0;
    def_scalefactor = 1.1;
    def_stridefactor = 0.1;
    def_minsize = 128;
    def_treshold = 5.0;
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

    for(std::list<classifier>::iterator it = m_classifiers.begin(); it != m_classifiers.end(); ++it){
        float rcsq[4*maxDetections];
        int ndetections = find_objects(rcsq, maxDetections, it->cascade, it->angle, pixels, nrows, ncols, ldim, it->scalefactor, it->stridefactor, it->minsize, MIN(nrows, ncols));

        if(ndetections){
            AL::ALValue out;
            for(int k = 0; k<ndetections; k++){
                if(rcsq[4*k + 3] < it->treshold) continue;
                AL::ALValue tmp;
                tmp.arrayPush(it->name);
                tmp.arrayPush(rcsq[4*k + 0]);
                tmp.arrayPush(rcsq[4*k + 1]);
                tmp.arrayPush(rcsq[4*k + 2]);
                tmp.arrayPush(rcsq[4*k + 3]);
                out.arrayPush(tmp);
                #ifdef MODULE_IS_REMOTE
                    cv::circle(imgOrig, cv::Point(static_cast<int>(rcsq[4*k + 1]),static_cast<int>(rcsq[4*k + 0])), static_cast<int>(rcsq[4*k + 2]/2), cv::Scalar( 0, 0, 255 ), 5);
                #endif
            }
            m_memoryProxy.raiseEvent("picoDetections", out);
        }
    }
    #ifdef MODULE_IS_REMOTE
            cv::imwrite("slika.png",imgOrig);
    #endif
}

void PicoModule::addClassifier(std::string name, std::string cascade, float angle, float scalefactor, float stridefactor, int minsize, int treshold){
    classifier temp;
    temp.name = name;
    temp.angle = angle < 0 ? def_angle : angle*2*3.1415926;
    temp.scalefactor = scalefactor < 0 ? def_scalefactor : scalefactor;
    temp.stridefactor = stridefactor < 0 ? def_stridefactor : stridefactor;
    temp.minsize = minsize < 0 ? def_minsize : minsize;
    temp.treshold = treshold < 0 ? def_treshold : treshold;

    FILE* file = fopen(cascade.c_str(), "rb");
    if(!file){
        std::cout << "[ERROR][PICO] Cannot read cascade from: " << cascade << std::endl;
        return;
    }
    fseek(file, 0L, SEEK_END);
    int size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    temp.cascade = malloc(size);
    if(!temp.cascade || size!=fread(temp.cascade, 1, size, file)){
        std::cout << "[ERROR][PICO] Failure while reading cascade from: " << cascade << std::endl;
        free(temp.cascade);
    } else m_classifiers.push_back(temp);
    fclose(file);
}

void PicoModule::removeClassifier(std::string name){
    for(std::list<classifier>::iterator it = m_classifiers.begin(); it != m_classifiers.end();)
        if(!name.compare(it->name)){
            free(it->cascade);
            m_classifiers.erase(it++);
        } else ++it;
}

void PicoModule::detectOnImage(AL::ALImage image){
    process(&image);
}
