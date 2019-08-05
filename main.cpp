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

    int ret = 0;
    char *outUrl = "rtmp://10.88.7.193/live";

    // 视频采集
    VideoCapture cam;
    Mat frame;

    /// 1 使用opencv打开rtsp相机
    cam.open(0);
    if (!cam.isOpened())
    {
        cout << "cam open failed!" << endl;
        return -1;
    }
    int inWidth = cam.get(CAP_PROP_FRAME_WIDTH);
    int inHeight = cam.get(CAP_PROP_FRAME_HEIGHT);
    int fps = cam.get(CAP_PROP_FPS);
    cout << "打开相机成功 inWidth="<<inWidth <<", inHeight="<< inHeight<<", fps=" <<fps << endl;

    XMediaEncode *xe = XMediaEncode::getInstance(0);
    ///2 初始化格式转换上下文, RGB->YUV
    ///3 初始化输出的数据结构
    xe->inWidth = inWidth;
    xe->inHeight = inHeight;
    xe->outWidth = inWidth;
    xe->outHeight = inHeight;
    xe->InitScale(); // 初始化Scale,用来做格式转换用

    ///4 初始化编码上下文
    //a 找到编码器
    if (!xe->InitVideoCodec()){
        cout << "InitVideoCodec failed!" << endl;
        return -1;
    }

    ////////////////////////////////////////////
    ///1 qt音频开始录制
    cout << "qt音频开始录制" << endl;
    // 音频采集参数
    int sampleRate = 44100;
    int channels = 2;
    int sampleByte = 2;
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


    ///2 音频重采样 上下文初始化   16bit转成FLATP
    xe->channels = channels;
    xe->nbSample = 1024;
    xe->sampleRate = sampleRate;
    xe->inSampleFMT = XSampleFMT::X_S16;
    xe->outSampleFMT = XSampleFMT::X_FLATP;
    if (!xe->InitResample())
    {
        cout << "InitResample 错误!" << endl;
        return -1;
    }
    ///4 初始化音频编码器
    if (!xe->InitAudioCodec())
    {
        cout << "InitAudioCodec 错误!" << endl;
        return -1;
    }

    //////////////////////////////////////////////////
    ///5 输出封装器和音频流配置
    //a 创建输出封装器上下文
    XRtmp *xr = XRtmp::getInstance(0);
    if (!xr->InitMux(outUrl))
    {
        cout << "InitMux 错误!" << endl;
        return -1;
    }
    // 添加音频流
    if (!xr->AddStream(xe->audioCodecContext))
    {
        cout << "AddStream audioCodecContext 错误!" << endl;
        return -1;
    }
    // 添加视频流
    if (!xr->AddStream(xe->videoCodecContext))
    {
        cout << "AddStream videoCodecContext 错误!" << endl;
        return -1;
    }

    ///打开rtmp 的网络输出IO
    //写入封装头
    if (!xr->SendMuxHead())
    {
        cout << "SendMuxHead 错误!" << endl;
        return -1;
    }

    //一次读取一帧音频的字节数
    int readSize = xe->nbSample*channels*sampleByte;
    char *buf = new char[readSize];

    cout << "------开始推流--------" << endl;
    for (;;)
    {

        ///读取rtsp视频帧，解码视频帧
        if (!cam.grab())
        {
            continue;
        }
        ///yuv转换为rgb
        if (!cam.retrieve(frame))
        {
            continue;
        }
        ///rgb to yuv
        xe->inPixSize = frame.elemSize();
        cout << " frame.elemSize = " << xe->inPixSize << endl;
        AVFrame *yuv = xe->RGBToYUV((unsigned  char*)frame.data);
        if (!yuv) continue;

        ///h264编码
        AVPacket *pack = xe->EncodeVideo(yuv);
        if (!pack) continue;

        xr->SendFrame(pack);



        //一次读取一帧音频  ----- 这样视频和音频放在一起推流是不能成功的！！！因为没设置stream_id
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

    return a.exec();
}
