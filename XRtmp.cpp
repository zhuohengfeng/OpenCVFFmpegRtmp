//
// Created by hengfeng zhuo on 2019-07-20.
//

#include "XRtmp.h"
#include "main.h"

using namespace std;

// 定义实现的子类
class CXRtmp : public XRtmp {

public:


    // 初始化封装器MUX上下文
    bool InitMux(const char* url) {
        // 输出封装器和视频流配置
        int ret = avformat_alloc_output_context2(&avFormatContext, 0, "flv", url);
        this->outURL = url;
        if (ret != 0) {
            qDebug() << "avformat_alloc_output_context2 出错";
            return false;
        }
        return true;
    }

    // 添加视频或者音频流---这里的codecContext是XMediaEncode中创建后传入
    bool AddStream(const AVCodecContext* codecContext) {
        if (!codecContext) {
            return false;
        }

        // 添加视频流
        AVStream* avStream = avformat_new_stream(avFormatContext, NULL);
        if (!avStream) {
            qDebug() << "avformat_new_stream 出错";
            return false;
        }

        avStream->codecpar->codec_tag = 0; //
        // 从编码器复制参数
        avcodec_parameters_from_context(avStream->codecpar, codecContext);
        av_dump_format(avFormatContext, 0, outURL.c_str(), 1);

        if (codecContext->codec_type == AVMEDIA_TYPE_VIDEO) {
            this->videoCodecContext = codecContext;
            this->videoStream = avStream;
        }
        else if (codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
            this->audioCodecContext = codecContext;
            this->audioStream = avStream;
        }
        return true;
    }

    // 打开RTMP网络IO，发送封装头MUX
    bool SendMuxHead() {
        ///打开rtmp 的网络输出IO
        int ret = avio_open(&avFormatContext->pb, outURL.c_str(), AVIO_FLAG_WRITE);
        if (ret != 0)
        {
            qDebug() << "avio_open 出错";
            return false;
        }

        //写入封装头
        ret = avformat_write_header(avFormatContext, NULL);
        if (ret != 0)
        {
            qDebug() << "avformat_write_header 出错";
            return false;
        }
        return true;
    }

    // RTMP推流
    bool SendFrame(AVPacket* pack) {
        if (pack->size <= 0 || !pack->data)return false;

        AVRational stime;
        AVRational dtime;

        //判断是音频还是视频
        if (videoStream && videoCodecContext && pack->stream_index == videoStream->index)
        {
            stime = videoCodecContext->time_base;
            dtime = videoStream->time_base;
        }
        else if (audioStream && audioCodecContext &&pack->stream_index == audioStream->index)
        {
            stime = audioCodecContext->time_base;
            dtime = audioStream->time_base;
        }
        else
        {
            return false;
        }

        //推流
        pack->pts = av_rescale_q(pack->pts, stime, dtime);
        pack->dts = av_rescale_q(pack->dts, stime, dtime);
        pack->duration = av_rescale_q(pack->duration, stime, dtime);
        int ret = av_interleaved_write_frame(avFormatContext, pack);
        if (ret == 0)
        {
            qDebug() << "SendFrame 出错";
            return true;
        }
        return false;
    }

    void Close() {
        if (avFormatContext) {
            avformat_close_input(&avFormatContext);
            avFormatContext = NULL;
        }
        videoStream = NULL;
        outURL = "";
    }

private:
    // RTMP FLV 封装器
    AVFormatContext* avFormatContext = NULL;

    // 视频编码器流
    const AVCodecContext *videoCodecContext = NULL;
    // 音频编码器流
    const AVCodecContext *audioCodecContext = NULL;

    // 新创建的视频流
    AVStream *videoStream = NULL;
    // 新创建的音频流
    AVStream *audioStream = NULL;

    string outURL = "";
};



XRtmp *XRtmp::getInstance(unsigned char index) {

    static CXRtmp rtmp[255];
    static bool isFirst = true;
    if (isFirst) {
        qDebug() << "XRtmp 首次启动初始化";
        av_register_all();
        avformat_network_init();
        isFirst = false;
    }

    return &rtmp[index];
}

XRtmp::XRtmp() {

}

XRtmp::~XRtmp() {

}