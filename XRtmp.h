//
// Created by hengfeng zhuo on 2019-07-20.
//

#ifndef OPENCVFFMPEGRTMP_XRTMP_H
#define OPENCVFFMPEGRTMP_XRTMP_H

// 为了编译过，不引入头文件
struct AVCodecContext;
struct AVPacket;

class XRtmp {

public:

    // 工厂方法
    static XRtmp* getInstance(unsigned char index = 0);

    // 初始化封装器MUX上下文
    virtual bool InitMux(const char* url) = 0;

    // 添加视频或者音频流
    virtual bool AddStream(const AVCodecContext* codecContext) = 0;

    // 打开RTMP网络IO，发送封装头MUX
    virtual bool SendMuxHead() = 0;

    // RTMP推流
    virtual bool SendFrame(AVPacket* pkt) = 0;

    virtual void Close() = 0;

    virtual ~XRtmp();

protected:
    XRtmp();
};


#endif //OPENCVFFMPEGRTMP_XRTMP_H
