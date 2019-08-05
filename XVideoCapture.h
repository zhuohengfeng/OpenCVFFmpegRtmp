//
// Created by hengfeng zhuo on 2019-08-05.
//

#ifndef OPENCVFFMPEGRTMP_XVIDEOCAPTURE_H
#define OPENCVFFMPEGRTMP_XVIDEOCAPTURE_H

#include "XDataThread.h"


class XVideoCapture : public XDataThread {

public:
    int width = 0;
    int height = 0;
    int fps = 0;
    static XVideoCapture *Get(unsigned char index = 0);
    virtual bool Init(int camIndex = 0) = 0; // 打开本地摄像头
    virtual bool Init(const char* url) = 0; // 打开流
    virtual void Stop() = 0;

    virtual ~XVideoCapture();

protected:
    XVideoCapture();

};


#endif //OPENCVFFMPEGRTMP_XVIDEOCAPTURE_H
