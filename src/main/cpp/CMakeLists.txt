project(PngView)

cmake_minimum_required(VERSION 3.4.1)

include_directories(include)

add_subdirectory(3rdparty/libpng)


add_library(PngView SHARED PngView.cpp
        PngRender.cpp)

find_library(OPENGLES3_LIBRARY GLESv3 "OpenGL ES v3.0 library")
find_library(EGL_LIBRARY EGL "EGL 1.4 library")
find_library(log-lib log)
find_library(android-lib android)

target_link_libraries(PngView
        ${log-lib}
        ${android-lib}
        ${OPENGLES3_LIBRARY}
        ${EGL_LIBRARY}
        pngdecoder)