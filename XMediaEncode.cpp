//
// Created by hengfeng zhuo on 2019-07-20.
//

#include "XMediaEncode.h"
#include "main.h"

using namespace std;

#if defined WIN32 || defined _WIN32
#include <windows.h>
#endif

/**
 * 获取CPU数量
 * @return
 */
static int XGetCpuNum()
{
#if defined WIN32 || defined _WIN32
    SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	return (int)sysinfo.dwNumberOfProcessors;
#elif defined __linux__
    return (int)sysconf(_SC_NPROCESSORS_ONLN);
#elif defined __APPLE__
    int numCPU = 0;
    int mib[4];
    size_t len = sizeof(numCPU);

//    // set the mib for hw.ncpu
//    mib[0] = CTL_HW;
//    mib[1] = HW_AVAILCPU;  // alternatively, try HW_NCPU;
//
//    // get the number of CPUs from the system
//    sysctl(mib, 2, &numCPU, &len, NULL, 0);
//
//    if (numCPU < 1)
//    {
//        mib[1] = HW_NCPU;
//        sysctl(mib, 2, &numCPU, &len, NULL, 0);
//
//        if (numCPU < 1)
//            numCPU = 1;
//    }
    return 2;//(int)numCPU;
#else
    return 1;
#endif
}



// 定义一个子类
class CXMediaEncode : public XMediaEncode {

public:

    /**
     * 初始化格式转换上下文 RGB->YUV
     * @return
     */
    bool InitScale() {
        swsContext = sws_getCachedContext(swsContext,
                                          inWidth, inHeight, AV_PIX_FMT_BGR24,
                                          outWidth, outHeight, AV_PIX_FMT_YUV420P,
                                          SWS_BICUBIC,
                                          0, 0, 0);

        if (!swsContext) {
            cout << "[Error] 初始化格式转换上下文sws_getCachedContext失败" << endl;
            return false;
        }
        // 初始化输出的数据结构，这个时候还没开始转换，内容是空的
        yuvAvFrame = av_frame_alloc();
        yuvAvFrame->format = AV_PIX_FMT_YUV420P;
        yuvAvFrame->width = inWidth;
        yuvAvFrame->height = inHeight;
        yuvAvFrame->pts = 0;
        // 实际分配yuv空间
        int ret = av_frame_get_buffer(yuvAvFrame, 0);
        if (ret != 0) {
            cout << "[Error] av_frame_get_buffer失败" << endl;
            return false;
        }
        cout << "XMediaEncode InitScale 完成"<< endl;
        return true;
    }

    /**
     * 开始进行像素格式转换
     * @param rgb
     * @return
     */
    AVFrame* RGBToYUV(unsigned char *rgb) {
        // rgb to yuv
        //输入的数据结构
        uint8_t *indata[AV_NUM_DATA_POINTERS] = {0};
        indata[0] = rgb;
        int insize[AV_NUM_DATA_POINTERS] = {0};
        // 一行(宽)数据的字节数
        insize[0] = inWidth * inPixSize;
        // 开始格式转换，把转换后的数据存放到yuvAvFrame->data中
        int h = sws_scale(swsContext, indata, insize, 0, inHeight,
                          yuvAvFrame->data, yuvAvFrame->linesize);
        if (h <= 0) {
            cout << "[Error]RGBToYUV h <= 0" << endl;
            return NULL;
        }

        return yuvAvFrame;
    }

    /**
     * 初始化编码器
     * @return
     */
    bool InitVideoCodec() {
        int ret = 0;
        // 找到编码器
        AVCodec* videoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
        if (!videoCodec) {
            cout << "InitVideoCodec 获取解码器出错" << endl;
            return false;
        }
        // 创建编码器上下文
        videoCodecContext = avcodec_alloc_context3(videoCodec);
        if (!videoCodecContext) {
            cout << "InitVideoCodec 获取解码器上下文出错" << endl;
            return false;
        }
        // 配置编码器参数
        videoCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        videoCodecContext->codec_id = videoCodec->id;
        videoCodecContext->thread_count = XGetCpuNum();
        cout << "InitVideoCodec 获取CUP NUM: " << videoCodecContext->thread_count << endl;
        //压缩后每秒视频的bit位大小 50kB
        videoCodecContext->bit_rate = 10 *1024 * 1024; //
        videoCodecContext->width = outWidth;
        videoCodecContext->height = outHeight;
        videoCodecContext->time_base = {1, fps};
        videoCodecContext->framerate = {fps, 1};
        // 画面组的大小，多少帧一个关键帧
        videoCodecContext->gop_size = 10;
        videoCodecContext->max_b_frames = 0;
        videoCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;

        // 打开编码器上下文
        ret = avcodec_open2(videoCodecContext, 0, 0);
        if (ret != 0) {
            cout << "avcodec_open2出错" << endl;
            return false;
        }

        cout << "XMediaEncode InitVideoCodec 完成"<< endl;
        return true;
    }

    /**
     * 开始编码
     * @param frame
     * @return
     */
    AVPacket* EncodeVideo(AVFrame* frame) {
        // 先释放空间
        av_packet_unref(&outVideoPacket);

        // 开始h264编码, pts必须递增
        frame->pts = videoPts;
        videoPts++;

        // 发送原始帧，开始编码
        int ret = avcodec_send_frame(videoCodecContext, frame);
        if (ret != 0) {
            return NULL;
        }

        ret = avcodec_receive_packet(videoCodecContext, &outVideoPacket);
        if (ret != 0 || outVideoPacket.size <= 0) {
            return NULL;
        }

        return &outVideoPacket;
    }


    ///////////////////////////////////////////////////////////////////////////
    bool InitResample()
    {
        ///2 音频重采样 上下文初始化
        swrContext = swr_alloc_set_opts(swrContext,
                                 av_get_default_channel_layout(channels), (AVSampleFormat)outSampleFMT, sampleRate,//输出格式
                                 av_get_default_channel_layout(channels), (AVSampleFormat)inSampleFMT, sampleRate, 0, 0);//输入格式
        if (!swrContext)
        {
            cout << "swrContext failed!"<< endl;
            return false;
        }
        int ret = swr_init(swrContext);
        if (ret != 0)
        {
            cout << "swr_init failed!"<< endl;
            return false;
        }

        ///3 音频重采样输出空间分配
        pcmAvFrame = av_frame_alloc();
        pcmAvFrame->format = outSampleFMT;
        pcmAvFrame->channels = channels;
        pcmAvFrame->channel_layout = av_get_default_channel_layout(channels);
        pcmAvFrame->nb_samples = nbSample; //一帧音频一通道的采用数量
        ret = av_frame_get_buffer(pcmAvFrame, 0); // 给pcm分配存储空间
        if (ret != 0)
        {
            cout << "av_frame_get_buffer failed!"<< endl;
            return false;
        }

        cout << "音频重采样 上下文初始化成功!"<< endl;
        return true;
    }


    AVFrame *Resample(unsigned char *data)
    {
        const uint8_t *indata[AV_NUM_DATA_POINTERS] = { 0 };
        indata[0] = (uint8_t *)data;
        int len = swr_convert(swrContext, pcmAvFrame->data, pcmAvFrame->nb_samples, //输出参数，输出存储地址和样本数量
                              indata, pcmAvFrame->nb_samples
        );
        if (len <= 0)
        {
            return NULL;
        }
        return pcmAvFrame;
    }

    bool InitAudioCodec()
    {
        if (!CreateCodec(AV_CODEC_ID_AAC))
        {
            return false;
        }
        audioCodecContext->bit_rate = 40000;
        audioCodecContext->sample_rate = sampleRate;
        audioCodecContext->sample_fmt = AV_SAMPLE_FMT_FLTP;
        audioCodecContext->channels = channels;
        audioCodecContext->channel_layout = av_get_default_channel_layout(channels);
        return OpenCodec(&audioCodecContext);
    }

    AVPacket * EncodeAudio(AVFrame* frame)
    {
        pcmAvFrame->pts = audioPts;
        audioPts += av_rescale_q(pcmAvFrame->nb_samples, { 1,sampleRate }, audioCodecContext->time_base);
        int ret = avcodec_send_frame(audioCodecContext, pcmAvFrame);
        if (ret != 0)
            return NULL;
        av_packet_unref(&outAudioPacket);
        ret = avcodec_receive_packet(audioCodecContext, &outAudioPacket);
        if (ret != 0)
            return NULL;
        return &outAudioPacket;
    }

    /**
     * 关闭资源
     */
    void Close() {
        if (swsContext) {
            sws_freeContext(swsContext);
            swsContext = NULL;
        }
        if (yuvAvFrame) {
            av_frame_free(&yuvAvFrame);
        }
        if (videoCodecContext) {
            avcodec_free_context(&videoCodecContext);
        }
        videoPts = 0;
        av_packet_unref(&outVideoPacket);
    }

private:
    // 音频
    SwrContext* swrContext = NULL;
    AVFrame* pcmAvFrame = NULL;
    AVPacket outAudioPacket = {0};

    // 视频
    SwsContext* swsContext = NULL; // 像素格式转换上下文
    AVFrame* yuvAvFrame = NULL; // 存放转换后的YUV数据
    AVPacket outVideoPacket = {0}; // 编码后的数据
    
    int videoPts = 0;
    int audioPts = 0;


    bool OpenCodec(AVCodecContext **c)
    {
        //打开音频编码器
        int ret = avcodec_open2(*c, 0, 0);
        if (ret != 0)
        {
            cout << "avcodec_open2 failed!"<< endl;
            return false;
        }
        cout << "avcodec_open2 success!"<< endl;
        return true;
    }

    bool CreateCodec(AVCodecID cid)
    {
        ///4 初始化编码器 AV_CODEC_ID_AAC
        AVCodec *codec = avcodec_find_encoder(cid);
        if (!codec)
        {
            cout << "avcodec_find_encoder failed!"<< endl;
            return false;
        }

        //音频编码器上下文
        audioCodecContext = avcodec_alloc_context3(codec);
        if (!audioCodecContext)
        {
            cout << "avcodec_alloc_context3 failed!"<< endl;
            return false;
        }
        audioCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        audioCodecContext->thread_count = XGetCpuNum();
        return true;
    }

};



/////////////////////////////////////////////////
XMediaEncode *XMediaEncode::getInstance(unsigned char index) {
    static bool isFirst = true; // 注意这里，可以直接在这里定义，也是只有一份
    if (isFirst) {
        cout << "XMediaEncode 首次启动初始化"<< endl;
        // 注册所有ffmpeg的编解码器
        av_register_all(); //注册FFmpeg所有编解码器。
        avcodec_register_all();
        isFirst = false;
    }

    static CXMediaEncode cxm[255]; // 这个也是静态的
    return &cxm[index];
}

XMediaEncode::XMediaEncode() {
}

XMediaEncode::~XMediaEncode() {
}