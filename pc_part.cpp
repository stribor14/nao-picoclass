#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <alcommon/alproxy.h>
#include <alproxies/alvideodeviceproxy.h>
#include <alproxies/almemoryproxy.h>
#include <alvision/alvisiondefinitions.h>

#include "boost/filesystem.hpp"
#include "APIwrappers.h"

int kbhit (void){
  struct timeval tv;
  fd_set rdfs;

  tv.tv_sec = 0;
  tv.tv_usec = 0;

  FD_ZERO(&rdfs);
  FD_SET (STDIN_FILENO, &rdfs);

  select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
  return FD_ISSET(STDIN_FILENO, &rdfs);

}

int main(int argc, char* argv[]) {

    boost::shared_ptr<AL::ALProxy> PicoModule = boost::shared_ptr<AL::ALProxy>(new AL::ALProxy("PicoModule", "herrflick.local", 9559));
    boost::shared_ptr<AL::ALProxy> memoryProxy = boost::shared_ptr<AL::ALProxy>(new AL::ALProxy("ALMemory", "herrflick.local", 9559));

    try {
        cv::Mat imgTemp;
        std::cout << "Acquiring image" << std::endl;
        AL::ALVideoDeviceProxy camProxy("herrflick.local", 9559);
        const std::string clientName = camProxy.subscribe("pc_part", AL::k4VGA, AL::kYuvColorSpace, 5);
        AL::ALValue pic = camProxy.getImageRemote(clientName);
        imgTemp = cv::Mat(cv::Size(pic[0], pic[1]), CV_8UC1);
        imgTemp.data = (uchar*) pic[6].GetBinary();
        camProxy.releaseImage(clientName);
        camProxy.unsubscribe(clientName);
        std::cout << "Sending image to classification" << std::endl;
        AL::ALValue oldHistory, newHistory;
        try {oldHistory = memoryProxy->call<AL::ALValue>("getEventHistory", "picoDetections");}
        catch (const AL::ALError& e){}
        PicoModule->callVoid("setParameters", "/home/mirko/Devel/ADORE/PicoClass/pico/rnt/cascades/facefinder", -1, -1, -1, -1, -1);
        PicoModule->callVoid("detectOnImage",MATtoAL(imgTemp));
        std::cout << "Press ENTER to end the process" << std::endl;
        while(!kbhit()){
            try {newHistory = memoryProxy->call<AL::ALValue>("getEventHistory", "picoDetections");}
            catch (const AL::ALError& e){}
            for (int k= oldHistory.getSize(); k < newHistory.getSize(); k++){
                std::vector<float> temp;
                newHistory[k].ToFloatArray(temp);
                std::cout << temp[0] << " " << temp[1] << " " << temp[2] << " " << temp[3] << std::endl;
            }
            oldHistory = newHistory;
        }
    }
    catch (const AL::ALError& e) {
      std::cerr << e.what() << std::endl;
    }
}
