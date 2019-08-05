//
// Created by hengfeng zhuo on 2019-08-05.
//

#ifndef OPENCVFFMPEGRTMP_XDATA_H
#define OPENCVFFMPEGRTMP_XDATA_H

/**
 * 用来专门存储音视频数据
 */
class XData {

public:
    char* data = 0;// 存放音视频数据
    int size = 0;
    void Drop();
    XData();
    XData(char* data, int size);
    virtual ~XData();
};


#endif //OPENCVFFMPEGRTMP_XDATA_H
