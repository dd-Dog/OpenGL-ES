project(libpngdecoder)

CMAKE_MINIMUM_REQUIRED(VERSION 3.4.1)
#MESSAGE(WARNING "CMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

SET(LOCAL_SRC_FILES
        example.c
        image.c
        png.c
        png_decoder.cpp
        pngerror.c
        pngget.c
        pngmem.c
        pngpread.c
        pngread.c
        pngrio.c
        pngrtran.c
        pngrutil.c
        pngset.c
        pngtrans.c
        pngwio.c
        pngwrite.c
        pngwtran.c
        pngwutil.c

        )
ADD_LIBRARY(pngdecoder SHARED ${LOCAL_SRC_FILES})

target_link_libraries(pngdecoder z) #链接系统libz库,必须的