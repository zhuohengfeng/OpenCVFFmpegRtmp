//
// Created by hengfeng zhuo on 2019-07-20.
//

#ifndef OPENCVFFMPEGRTMP_XMEDIAENCODE_H
#define OPENCVFFMPEGRTMP_XMEDIAENCODE_H

struct AVFrame; // 注意，这里只是声明，保证编译可以通过。而不需要引入头文件
struct AVPacket;
struct AVCodecContext;

/**
 * 编解码类, 用工厂类来实现
 * 注意这里不应该包括其他比如FFMpeg等的头文件，因为如果我们生产一个动态库，只需要把我们封装后的头文件提供就好了
 * 并且具体的实现我们都是用接口(纯虚函数的方式来实现)
 *
 * 1. 对opencv采集的RGB数据转换成YUV，
 * 2. 然后编码成H264
 *
 *
 * AVRational time_base：时基。通过该值可以把PTS，DTS转化为真正的时间。
 * FFMPEG其他结构体中也有这个字段，但是根据经验，只有AVStream中的time_base是可用的。PTS*time_base=真正的时间
 *
 * 例如对于H.264来说。1个AVPacket的data通常对应一个NAL。
 */


enum XSampleFMT {
    X_S16 = 1,
    X_FLATP = 8
};

class XMediaEncode {

public:
    // 输入参数
    int inWidth = 1280;
    int inHeight = 720;
    int inPixSize = 3; //BGR
    // 音频
    int channels = 2;
    int sampleRate = 44100;
    XSampleFMT inSampleFMT = X_S16;

    // 输出参数
    int outWidth = 1280;
    int outHeight = 720;
    int bitRate = 50 * 1024 * 8;// 压缩后每秒视频的bit位大小 50KB
    int fps = 25;
    // 音频
    int nbSample = 1024;
    XSampleFMT outSampleFMT = X_FLATP;

    // 编码器上下文， YUV->H264
    AVCodecContext* videoCodecContext = nullptr;
    // 音频编码上下文, PCM-AAC
    AVCodecContext* audioCodecContext = nullptr;

    // 工厂生产方法
    static XMediaEncode* getInstance(unsigned char index = 0); // 静态方法，获取实例; 有可能有多个实例，所以这里传入一个索引


    // 初始化像素格式转换的上下文初始化
    virtual bool InitScale() = 0; //纯虚函数
    // 开始格式转换
    virtual AVFrame* RGBToYUV(unsigned char *rgb) = 0;
    // 视频编码器初始化
    virtual bool InitVideoCodec() = 0;
    // 开始编码视频
    virtual AVPacket* EncodeVideo(AVFrame* frame) = 0;

    // 对于音频来说, 音频重采样上下文初始化
    virtual bool InitResample() = 0;
    // 重采样
    virtual AVFrame* Resample(unsigned char *pcm) = 0;
    // 音频编码器初始化
    virtual bool InitAudioCodec() = 0;
    // 开始音频编码
    virtual AVPacket* EncodeAudio(AVFrame* frame) = 0;

    // 关闭释放资源
    virtual void Close() = 0;

    virtual ~XMediaEncode();

protected:
    XMediaEncode(); // 把构造函数放在protected中，这样就不能直接创建实例

};

#endif //OPENCVFFMPEGRTMP_XMEDIAENCODE_H
