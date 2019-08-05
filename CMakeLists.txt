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

#--------------- QT5 ----------------------
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/Users/hengfeng/tools/Qt5.9.8/5.9.8/clang_64/lib/cmake")

#打开Qt的MOC功能和UI文件处理功能
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Sql REQUIRED)
find_package(Qt5Multimedia REQUIRED)

#--------------------------------------------
add_executable(OpenCVFFmpegRtmp
        main.cpp
        dialog.cpp
        XMediaEncode.cpp
        XRtmp.cpp XData.cpp XData.h XDataThread.cpp XDataThread.h XVideoCapture.cpp XVideoCapture.h XAudioRecord.cpp XAudioRecord.h)

# 链接库文件
TARGET_LINK_LIBRARIES(OpenCVFFmpegRtmp
        avcodec avdevice avfilter avformat avresample avutil postproc swresample swscale
        ${OpenCV_LIBS}
        Qt5::Core Qt5::Widgets Qt5::Sql Qt5::Multimedia
        )