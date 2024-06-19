# 添加Google Test
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# 为了编译不出错需要禁用gtest内部测试
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
# 启用测试
enable_testing()
# 添加测试可执行文件
add_executable(runTests test_utils.cpp)
# 链接Google Test和被测试的库
target_link_libraries(runTests gtest_main VideoPush)
# 添加测试
add_test(NAME runTests COMMAND runTests)