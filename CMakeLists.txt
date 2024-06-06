cmake_minimum_required(VERSION 3.20)
project(ffmpeg2cv)


set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR})

if (WIN32)
    set(FFMPEG_INCLUDE_DIRS "E:/develop/ffmpeg/include")
    set(FFMPEG_LIBRARY_DIRS "E:/develop/ffmpeg/lib")
else ()
    set(FFMPEG_INCLUDE_DIRS "/usr/local/Cellar/ffmpeg/6.1.1_2/include")
    set(FFMPEG_LIBRARY_DIRS "/usr/local/Cellar/ffmpeg/6.1.1_2/lib")
    # 其他系统
endif ()

include(E:/Fastdeploy/build/install/FastDeploy.cmake)
# 添加FastDeploy依赖头文件
include_directories(${FASTDEPLOY_INCS})


set(FFMPEG_LIBRARIES
        avdevice
        avfilter
        avformat
        avcodec
        swresample
        swscale
        avutil)

if (MSVC)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif ()

aux_source_directory(src SRC)

add_executable(${PROJECT_NAME} ${SRC} main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} ${FFMPEG_LIBRARIES} ${FASTDEPLOY_LIBS})