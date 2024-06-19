cmake_minimum_required(VERSION 3.20)
project(ffmpeg2cv)


option(EXPORT_CAPI "build c api" ON)
option(BUILD_TESTS "build test" ON)
set(FAST_DEPLOY_PATH "E:/Fastdeploy/build/install" CACHE PATH "fastdeploy path")
# macos /usr/local/Cellar/ffmpeg/6.1.1_2
set(FFMPEG_PATH "E:/develop/ffmpeg" CACHE PATH "ffmpeg path")


set(FFMPEG_INCLUDE_DIRS "${FFMPEG_PATH}/include")
set(FFMPEG_LIBRARY_DIRS "${FFMPEG_PATH}/lib")
if (WIN32)
    if (MSVC)
        add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
        add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    endif ()
endif ()

include(${FAST_DEPLOY_PATH}/FastDeploy.cmake)
set(CMAKE_CXX_STANDARD 17)
# 添加FastDeploy依赖头文件
include_directories(${FASTDEPLOY_INCS})
include_directories(${CMAKE_SOURCE_DIR})
set(FFMPEG_LIBRARIES
        avdevice
        avfilter
        avformat
        avcodec
        swresample
        swscale
        avutil
        )

set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

file(GLOB CPPSOURCES ${SOURCE_DIR}/*.h ${SOURCE_DIR}/*.cpp)
list(APPEND SOURCES ${CPPSOURCES})
if (EXPORT_CAPI)
    add_definitions(-DCAPI)
    file(GLOB CSOURCES capi/*.h capi/*.cpp)
    list(APPEND SOURCES ${CSOURCES})
endif ()

add_definitions(-DVP)
add_library(VideoPush SHARED ${SOURCES})
target_include_directories(VideoPush PUBLIC ${FFMPEG_INCLUDE_DIRS})
target_link_directories(VideoPush PUBLIC ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(VideoPush ${FFMPEG_LIBRARIES} ${FASTDEPLOY_LIBS})


add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} VideoPush)

if (BUILD_TESTS)
#    add_subdirectory(third_party/googletest)
    add_subdirectory(test)
#    # 添加Google Test
endif ()