## QT5 + Opencv + FFMpeg 采集视频后RTMP推流可视化应用

### 一. 方案

Opencv采集RTSP或者系统相机视频帧RGB
=> 通过FFMPEG转换成YUV格式 
=> YUV数据编码成H264
=> 编码后的数据推流到RTMP服务器

### 二. 架构

#### 1. opencv

#### 2. FFmepg

#### 3. QT5



### 三. 测试

#### 1. 搭建本地RTMP服务器(ubuntu nginx+rtmp)
```
git clone https://github.com/arut/nginx-rtmp-module.git
wget http://nginx.org/download/nginx-1.13.3.tar.gz
tar -zxvf nginx-1.13.3.tar.gz
cd nginx*
./configure --add-module=/root/rtmp/nginx-rtmp-module 
make
sudo make install

pkill nginx
sudo /usr/local/nginx/sbin/nginx

ps -ef | grep nginx
```

#### 2. 使用ffplay拉流测试
ffplay rtmp://10.88.7.193/live  -fflags nobuffer