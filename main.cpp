//
// Created by hengfeng zhuo on 2019-07-20.
//
#include "main.h"
#include "dialog.h"
#include <iostream>
#include "XMediaEncode.h"
#include "XRtmp.h"

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w;
    w.show();



    char *outUrl = "rtmp://192.168.1.44/live";
    int ret = 0;
    int sampleRate = 44100;
    int channels = 2;
    int sampleByte = 2;

    ///1 qt音频开始录制
    QAudioFormat fmt;
    fmt.setSampleRate(sampleRate);
    fmt.setChannelCount(channels);
    fmt.setSampleSize(sampleByte *8);
    fmt.setCodec("audio/pcm");
    fmt.setByteOrder(QAudioFormat::LittleEndian);
    fmt.setSampleType(QAudioFormat::UnSignedInt);
    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    if (!info.isFormatSupported(fmt))
    {
        cout << "Audio format not support!" << endl;
        fmt = info.nearestFormat(fmt);
    }
    QAudioInput *input = new QAudioInput(fmt);
    //开始录制音频
    QIODevice *io = input->start();


    ///2 音频重采样 上下文初始化
    XMediaEncode *xe = XMediaEncode::getInstance(0);
    xe->channels = channels;
    xe->nbSample = 1024;
    xe->sampleRate = sampleRate;
    xe->inSampleFMT = XSampleFMT::X_S16;
    xe->outSampleFMT = XSampleFMT::X_FLATP;
    if (!xe->InitResample())
    {
        getchar();
        return -1;
    }
    ///4 初始化音频编码器
    if (!xe->InitAudioCodec())
    {
        getchar();
        return -1;
    }

    ///5 输出封装器和音频流配置
    //a 创建输出封装器上下文
    XRtmp *xr = XRtmp::getInstance(0);
    if (!xr->InitMux(outUrl))
    {
        getchar();
        return -1;
    }
    //b 添加音频流
    if (!xr->AddStream(xe->audioCodecContext))
    {
        getchar();
        return -1;
    }

    ///打开rtmp 的网络输出IO
    //写入封装头
    if (!xr->SendMuxHead())
    {
        getchar();
        return -1;
    }
    //一次读取一帧音频的字节数
    int readSize = xe->nbSample*channels*sampleByte;
    char *buf = new char[readSize];
    for (;;)
    {
        //一次读取一帧音频
        if (input->bytesReady() < readSize)
        {
            QThread::msleep(1);
            continue;
        }
        int size = 0;
        while (size != readSize)
        {
            int len = io->read(buf + size, readSize - size);
            if (len < 0)break;
            size += len;
        }

        if (size != readSize)continue;

        //已经读一帧源数据
        //cout << size << " ";
        //重采样源数据
        AVFrame *pcm = xe->Resample((unsigned char*)buf);

        //pts 运算
        //  nb_sample/sample_rate  = 一帧音频的秒数sec
        // timebase  pts = sec * timebase.den
        AVPacket *pkt = xe->EncodeAudio(pcm);
        if (!pkt)continue;
        ////推流
        xr->SendFrame(pkt);
        if (ret == 0)
        {
            cout << "#" << flush;
        }

    }
    delete buf;

    getchar();
    return a.exec();
}
