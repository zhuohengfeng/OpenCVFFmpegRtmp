//
// Created by hengfeng zhuo on 2019-08-05.
//

#include <QAudioInput>
#include "XAudioRecord.h"
#include <iostream>

class CXAudioRecord : public XAudioRecord {

public:
    bool Init() override {
        this->Stop();
        // qt音频开始录制
        QAudioFormat fmt;
        fmt.setSampleRate(sampleRate);
        fmt.setChannelCount(channels);
        fmt.setSampleSize(sampleByte * 8);
        fmt.setCodec("audio/pcm");
        fmt.setByteOrder(QAudioFormat::LittleEndian);
        fmt.setSampleType(QAudioFormat::UnSignedInt);
        QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
        if (!info.isFormatSupported(fmt)) {
            std::cout << "Audio format not support" << std::endl;
            fmt = info.nearestFormat(fmt);
        }
        input = new QAudioInput(fmt);
        //开始录制音频
        io = input->start();
        if (!io)
            return false;
        return true;
    }

    void Stop() override {
        // 停止线程
        XDataThread::Stop();
        if (input)
            input->stop();
        if (io)
            io->close();
        input = NULL;
        io = NULL;
    }


protected:
    QAudioInput* input = nullptr;
    QIODevice* io = nullptr;

    /**
     * 这个是QThread的线程执行体
     */
    void run() override {
        std::cout << "进入音频录制线程" << std::endl;
        // 每次读取的字节数
        int readSize = nbSamples * channels * sampleByte;
        char* buf = new char[readSize];
        while(!this->isExit) {
            //读取已录制音频
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
            if (size != readSize)
            {
                continue;
            }
            //已经读取一帧音频
            Push(XData(buf, readSize));
        }
        delete[] buf;
        std::cout << "退出音频录制线程" << std::endl;
    }
};



XAudioRecord *XAudioRecord::Get(XAUDIOTYPE type, unsigned char index)
{
    static CXAudioRecord record[255];
    return &record[index];
}

XAudioRecord::XAudioRecord() {

}

XAudioRecord::~XAudioRecord() {

}
