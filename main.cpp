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

    // RTSP URL
    char* inUrl = "rtsp://184.72.239.149/vod/mp4://BigBuckBunny_175k.mov";
    // 直播服务的RTMP推流URL，这里也可以是一个Mux的文件格式，那就是直接保存到文件中
    char* outUrl = "rtmp://10.88.7.193/live";


    //编码器和像素格式转换
    XMediaEncode *me = XMediaEncode::getInstance(0);

    //封装和推流对象
    XRtmp *xr = XRtmp::getInstance(0);

    VideoCapture cam;
    Mat frame;
    namedWindow("video");

    int ret = 0;
    try
    {	////////////////////////////////////////////////////////////////
        /// 1 使用opencv打开rtsp相机
        cam.open(inUrl);
        if (!cam.isOpened())
        {
            throw "cam open failed!";
        }
        cout << inUrl << " cam open success" << endl;
        int inWidth = cam.get(CAP_PROP_FRAME_WIDTH);
        int inHeight = cam.get(CAP_PROP_FRAME_HEIGHT);
        int fps = cam.get(CAP_PROP_FPS);

        ///2 初始化格式转换上下文
        ///3 初始化输出的数据结构
        me->inWidth = inWidth;
        me->inHeight = inHeight;
        me->outWidth = inWidth;
        me->outHeight = inHeight;
        me->InitScale();

        ///4 初始化编码上下文
        //a 找到编码器
        if (!me->InitVideoCodec())
        {
            throw "InitVideoCodec failed!";
        }

        ///5 输出封装器和视频流配置
        xr->InitMux(outUrl);

        //添加视频流
        xr->AddStream(me->videoCodecContext);
        xr->SendMuxHead();

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
            //imshow("video", frame);
            //waitKey(1);


            ///rgb to yuv
            me->inPixSize = frame.elemSize();
            AVFrame *yuv = me->RGBToYUV((unsigned char*)frame.data);
            if (!yuv) continue;

            ///h264编码
            AVPacket *pack = me->EncodevIDEO(yuv);
            if (!pack) continue;

            xr->SendFrame(pack);
        }

    }
    catch (exception &ex)
    {
        if (cam.isOpened())
            cam.release();
        cerr << ex.what() << endl;
    }
    getchar();

    return a.exec();
}
