//
// Created by hengfeng zhuo on 2019-07-20.
//
#include "main.h"
#include "dialog.h"
#include <iostream>
#include "XMediaEncode.h"
#include "XRtmp.h"
#include "XAudioRecord.h"
#include "XVideoCapture.h"

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    int ret = 0;
    char *outUrl = "rtmp://10.88.7.193/live";

    int sampleRate = 44100;
    int channels = 2;
    int sampleByte = 2;
    int nbSample = 1024;

    ///打开摄像机
    XVideoCapture *xv = XVideoCapture::Get();
    if (!xv->Init(0))
    {
        cout << "open camera failed!" << endl;
        getchar();
        return -1;
    }
    cout << "open camera success!" << endl;
    xv->Start();

    ///1 qt音频开始录制
    XAudioRecord *ar = XAudioRecord::Get();
    ar->sampleRate = sampleRate;
    ar->channels = channels;
    ar->sampleByte = sampleByte;
    ar->nbSamples = nbSample;
    if (!ar->Init())
    {
        cout << "XAudioRecord Init failed!" << endl;
        getchar();
        return -1;
    }
    ar->Start();

    ///音视频编码类
    XMediaEncode *xe = XMediaEncode::getInstance();

    ///2 初始化格式转换上下文
    ///3 初始化输出的数据结构
    xe->inWidth = xv->width;
    xe->inHeight = xv->height;
    xe->outWidth = xv->width;
    xe->outHeight = xv->height;
    if (!xe->InitScale())
    {
        getchar();
        return -1;
    }
    cout << "初始化视频像素转换上下文成功!" << endl;

    ///2 音频重采样 上下文初始化
    xe->channels = channels;
    xe->nbSample = nbSample;
    xe->sampleRate = sampleRate;
    xe->inSampleFMT = XSampleFMT::X_S16;
    xe->outSampleFMT= XSampleFMT::X_FLATP;
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

    ///初始化视频编码器
    if (!xe->InitVideoCodec())
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

    //添加视频流
    int vindex = 0;
    vindex = xr->AddStream(xe->videoCodecContext);
    if (vindex<0)
    {
        getchar();
        return -1;
    }

    //b 添加音频流
    int aindex = xr->AddStream(xe->audioCodecContext);
    if (aindex<0)
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
    for (;;)
    {
        //一次读取一帧音频
        XData ad = ar->Pop();
        XData vd = xv->Pop();
        if (ad.size<=0 && vd.size<=0)
        {
            QThread::msleep(1);
            continue;
        }

        //处理音频
        if (ad.size > 0)
        {
            //重采样源数据
            AVFrame *pcm = xe->Resample((unsigned char *)(ad.data));
            ad.Drop();
            AVPacket *pkt = xe->EncodeAudio(pcm);
            if (pkt)
            {
                ////推流
                if (xr->SendFrame(pkt,aindex))
                {
                    cout << "#" << flush;
                }
            }

        }

        //处理视频
        if (vd.size > 0)
        {
            AVFrame *yuv = xe->RGBToYUV((unsigned char *)(vd.data));
            vd.Drop();
            AVPacket *pkt = xe->EncodeVideo(yuv);
            if (pkt)
            {
                ////推流
                if (xr->SendFrame(pkt,vindex))
                {
                    cout << "@" << flush;
                }
            }

        }



    }

    getchar();
    return a.exec();
}
