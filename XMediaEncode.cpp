//
// Created by hengfeng zhuo on 2019-07-20.
//

#include "XMediaEncode.h"
#include "main.h"


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
    return (int)numCPU;
#else
    return 1;
#endif
}



// 定义一个子类
class CXMediaEncode : public XMediaEncode {

public:

    /**
     * 初始化格式转换上下文
     * @return
     */
    bool InitScale() {
        swsContext = sws_getCachedContext(swsContext,
                                          inWidth, inHeight, AV_PIX_FMT_BGR24,
                                          outWidth, outHeight, AV_PIX_FMT_YUV420P,
                                          SWS_BICUBIC,
                                          0, 0, 0);

        if (!swsContext) {
            qDebug() << "[Error] 初始化格式转换上下文sws_getCachedContext失败";
            return false;
        }
        // 初始化输出的数据结构，这个时候还没开始转换，内容是空的
        yuvAvFrame = av_frame_alloc();
        yuvAvFrame->format = AV_PIX_FMT_YUV420P;
        yuvAvFrame->width = inWidth;
        yuvAvFrame->height = inHeight;
        yuvAvFrame->pts = 0;
        // 实际分配yuv空间
        int ret = av_frame_get_buffer(yuvAvFrame, 32);
        if (ret != 0) {
            qDebug() << "[Error] av_frame_get_buffer失败";
            return false;
        }

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
            qDebug() << "获取解码器出错";
            return false;
        }
        // 创建编码器上下文
        videoCodecContext = avcodec_alloc_context3(videoCodec);
        if (!videoCodecContext) {
            qDebug() << "获取解码器上下文出错";
            return false;
        }
        // 配置编码器参数
        videoCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        videoCodecContext->codec_id = videoCodec->id;
        videoCodecContext->thread_count = XGetCpuNum();
        //压缩后每秒视频的bit位大小 50kB
        videoCodecContext->bit_rate = 50 *1024 * 8; //
        videoCodecContext->width = outWidth;
        videoCodecContext->height = outHeight;
        videoCodecContext->time_base = {1, fps};
        videoCodecContext->framerate = {fps, 1};
        // 画面组的大小，多少帧一个关键帧
        videoCodecContext->gop_size = 50;
        videoCodecContext->max_b_frames = 0;
        videoCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;

        // 打开编码器上下文
        ret = avcodec_open2(videoCodecContext, 0, 0);
        if (ret != 0) {
            qDebug() << "avcodec_open2出错";
            return false;
        }

        return true;
    }

    /**
     * 开始编码
     * @param frame
     * @return
     */
    AVPacket* EncodevIDEO(AVFrame* frame) {
        // 先释放空间
        av_packet_unref(&outAvPacket);

        // 开始h264编码, pts必须递增
        frame->pts = vpts;
        vpts++;

        // 发送原始帧，开始编码
        int ret = avcodec_send_frame(videoCodecContext, frame);
        if (ret != 0) {
            return NULL;
        }

        ret = avcodec_receive_packet(videoCodecContext, &outAvPacket);
        if (ret != 0 || outAvPacket.size <= 0) {
            return NULL;
        }

        return &outAvPacket;
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
        vpts = 0;
        av_packet_unref(&outAvPacket);
    }

private:
    SwsContext* swsContext = NULL; // 像素格式转换上下文
    AVFrame* yuvAvFrame = NULL; // 存放转换后的YUV数据
    AVPacket outAvPacket = {0}; // 编码后的数据
    int vpts = 0;
};



/////////////////////////////////////////////////
XMediaEncode *XMediaEncode::getInstance(unsigned char index) {
    static bool isFirst = true; // 注意这里，可以直接在这里定义，也是只有一份
    if (isFirst) {
        qDebug() << "XMediaEncode 首次启动初始化";
        // 注册所有ffmpeg的编解码器
        av_register_all();
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