## Opencv + FFMpeg 采集视频后RTMP推流

### 1. 实现方案

=> Opencv采集RTSP或者系统相机视频帧RGB
=> 通过FFMPEG转换成YUV格式 
=> YUV数据编码成H264
=> 编码后的数据推流到RTMP服务器

### 2. 架构


### 3. 测试