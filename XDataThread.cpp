//
// Created by hengfeng zhuo on 2019-08-05.
//

#include "XDataThread.h"

XDataThread::XDataThread() {

}

XDataThread::~XDataThread() {

}

bool XDataThread::Start() {
    this->isExit = false;
    // 启动QT线程
    QThread::start();
    return true;
}


// wait()阻塞当前的进程，直到满足如下两个条件之一：
// 1.相关的线程完成其任务，然后如果线程已经结束，则该函数返回true，如果线程没有启动，则该函数也会返回true。
// 2. 经过了特定长度的时间，如果时间是ULONG_MAX（默认值），那么wait()函数几乎不会超时。
// （即该函数必须从run()函数返回）如果wait函数超时，那么该函数会返回false。
void XDataThread::Stop() {
    this->isExit = true;
    wait(); // 等待退出
}

void XDataThread::Push(XData d) {
    mutex.lock();
    // 如果数据超出大小了，丢弃数据
    if (datas.size() > maxList) {
        datas.front().Drop();
        datas.pop_front(); // 取出队列第一个元素
    }
    // 存入数据
    datas.push_back(d); // 存入到队列最后
    mutex.unlock();
}

XData XDataThread::Pop() {
    mutex.lock();
    if (datas.empty()) {
        // 如果是空的，就返回一个空数据
        mutex.unlock();
        return XData();
    }
    XData d = datas.front();
    datas.pop_front();
    mutex.unlock();
    return d;
}
