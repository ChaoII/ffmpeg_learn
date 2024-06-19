//
// Created by aichao on 2024/6/18.
//
//
#include <gtest/gtest.h>
#include "src/ai_utils.h"

int add(int x, int y) {
    return x + y;
}

//其中第一个参数是测试套件名称，第二个参数是测试用例名称，二者都必须是合法的C++标识符，并且不应该包含下划线。
TEST(TestAdd, test_add_1) {
    int result = add(22, 33);
    EXPECT_EQ(result, 55);
}

TEST(TestAdd, test_add_2) {
    int result = add(22, 11);
    EXPECT_EQ(result, 33);
}


TEST(TestAddC, test_add_3) {
    int result = add(22, 11);
    EXPECT_EQ(result, 33);
}

//TEST(TESTSTARTWITH, test_start_with) {
//    bool result = start_with("rttscsss", "rtts");
//    EXPECT_EQ(result, true);
//}

