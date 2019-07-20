cmake_minimum_required(VERSION 3.13)

project(OpenCVFFmpegRtmp)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "-g -Wall")

#--------------- FFMpeg ----------------------
set(FFMPEG_PATH "/usr/local/Cellar/ffmpeg/4.1.1")
INCLUDE_DIRECTORIES(${FFMPEG_PATH}/include)
LINK_DIRECTORIES(${FFMPEG_PATH}/lib)

#--------------- OpenCV ----------------------
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(OpenCVFFmpegRtmp
        main.cpp XMediaEncode.cpp XMediaEncode.h XRtmp.cpp XRtmp.h)

TARGET_LINK_LIBRARIES(OpenCVFFmpegRtmp
        avcodec avdevice avfilter avformat avresample avutil postproc swresample swscale
        ${OpenCV_LIBS}
        )