#ifndef API_WRAPPERS_H
#define API_WRAPPERS_H

#include <alvalue/alvalue.h>
#include <opencv2/core/core.hpp>

AL::ALValue MATtoAL(cv::Mat tempMat){
        AL::ALValue tempAl;

        tempAl.arrayPush (AL::ALValue (tempMat.rows));
        tempAl.arrayPush (AL::ALValue (tempMat.cols));
        tempAl.arrayPush (AL::ALValue (tempMat.type()));
        tempAl.arrayPush (AL::ALValue (static_cast <const void*> (tempMat.data), sizeof(uchar) * tempMat.cols * tempMat.rows * tempMat.channels()));

        return tempAl;
    }
AL::ALValue SEGtoAL(std::vector<cv::Point> tempSeg){
        std::vector<int> temp;
        for (int k=0;k<tempSeg.size();k++) {
                temp.push_back(tempSeg[k].x);
                temp.push_back(tempSeg[k].y);
            }
        return AL::ALValue (temp);
    }
AL::ALValue SEGntoAL(std::vector<std::vector<cv::Point> > tempSeg){
        std::vector<int> temp;
        for (int i=0; i<tempSeg.size();i++)
            for (int k=0;k<4;k++) {
                    temp.push_back(tempSeg[i][k].x);
                    temp.push_back(tempSeg[i][k].y);
                }
        return AL::ALValue (temp);
    }
AL::ALValue BOOLtoAL(bool tempBool){
        return AL::ALValue (tempBool);
    }
AL::ALValue INTtoAL(int tempInt){
        return AL::ALValue (tempInt);
    }
AL::ALValue STRINGtoAL(std::string tempBool){
        return AL::ALValue (tempBool);
    }

cv::Mat ALtoMAT(AL::ALValue tempAl){
        cv::Mat temp = cv::Mat(tempAl[0], tempAl[1], tempAl[2],(uchar*)tempAl[3].GetBinary());
        return temp.clone();
    }
std::vector<cv::Point> ALtoSEG(AL::ALValue tempAl){
        std::vector<cv::Point> temp;
        for (int k=0;k<4;k++) temp.push_back(cv::Point(tempAl[k*2], tempAl[k*2+1]));
        return temp;
    }
std::vector<std::vector<cv::Point> > ALtoSEGn(AL::ALValue tempAl){
        std::vector<std::vector<cv::Point> > temp1;
        for(int i=0;i < tempAl.getSize()/8 ; i++){
            std::vector<cv::Point> temp2;
            for (int k=0;k<4;k++) temp2.push_back(cv::Point(tempAl[i*8 + k*2], tempAl[i*8 + k*2+1]));
            temp1.push_back(temp2);
            }
        return temp1;
    }
bool ALtoBOOL(AL::ALValue tempAl){
        return tempAl;
    }
int ALtoINT(AL::ALValue tempAl){
        return tempAl;
    }
std::string ALtoSTRING(AL::ALValue tempAl){
        return (std::string) tempAl;
    }




#endif // API_WRAPPERS_H


