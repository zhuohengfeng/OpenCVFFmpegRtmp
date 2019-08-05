//
// Created by hengfeng zhuo on 2019-08-05.
//

#ifndef OPENCVFFMPEGRTMP_XDATATHREAD_H
#define OPENCVFFMPEGRTMP_XDATATHREAD_H

#include "XData.h"
#include <list>
#include <QThread>

/**
 * 管理数据的线程类
 */
class XDataThread : public QThread{

public:
    XDataThread();
    virtual ~XDataThread();

    // 列表最大值，超出就删除旧的数据
    int maxList = 100;

    // 在列表结尾插入
    virtual void Push(XData d);

    // 读取列表中最早的数据，返回数据需要调用XData.Drop清理
    virtual XData Pop();

    // 启动线程
    virtual bool Start();

    // 退出线程，并等待线程退出-阻塞
    virtual void Stop();

protected:
    //存放交互数据 插入策略 先进先出
    std::list<XData> datas;

    // 交互数据列表大小
    int dataCount = 0;

    // 互斥访问datas
    QMutex mutex;

    // 处理线程退出的标志
    bool isExit = false;
};


#endif //OPENCVFFMPEGRTMP_XDATATHREAD_H
