//
// Created by hengfeng zhuo on 2019-08-05.
//

#ifndef OPENCVFFMPEGRTMP_XAUDIORECORD_H
#define OPENCVFFMPEGRTMP_XAUDIORECORD_H

#include "XDataThread.h"

enum XAUDIOTYPE {
    X_AUDIO_QT // 使用QT进行录音
};

class XAudioRecord : public XDataThread{

public:
    int channels = 2; // 声道数
    int sampleRate = 44100; // 采样率
    int sampleByte = 2; // 采样字节数(2字节，16位)
    int nbSamples = 1024; // 一帧音频每个通道的采样数量

    // 开始录制
    virtual bool Init() = 0;
    // 停止录制
    virtual void Stop() = 0;

    static XAudioRecord* Get(XAUDIOTYPE type = X_AUDIO_QT, unsigned char index = 0);

    virtual ~XAudioRecord();

protected:
    XAudioRecord();

};


#endif //OPENCVFFMPEGRTMP_XAUDIORECORD_H
