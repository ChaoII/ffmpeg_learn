cmake_minimum_required(VERSION 3.26)
project(ffmpeg2cv)


set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})

set(FFMPEG_INCLUDE_DIRS "E:/develop/ffmpeg-n7.0-latest-win64-gpl-shared-7.0/include")
set(FFMPEG_LIBRARY_DIRS "E:/develop/ffmpeg-n7.0-latest-win64-gpl-shared-7.0/lib")
set(FFMPEG_LIBRARIES avdevice
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

message("${SRC}")

add_executable(${PROJECT_NAME} ${SRC} main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${FFMPEG_LIBRARIES})