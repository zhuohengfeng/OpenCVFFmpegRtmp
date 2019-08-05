//
// Created by hengfeng zhuo on 2019-08-05.
//

#include <cstring>
#include "XData.h"

XData::XData() {

}

/**
 * 分配空间
 * @param data
 * @param size
 */
XData::XData(char *data, int size) {
    this->data = new char[size];
    memcpy(this->data, data, size);
    this->size = size;
}

XData::~XData() {

}

/**
 * 释放数据
 */
void XData::Drop() {
    if (this->data) {
        delete this->data;
    }
    this->data = nullptr;
    this->size = 0;
}
