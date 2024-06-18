//
// Created by AC on 2024/6/12.
//

#include "log.h"

bool VPLogger::enable_info = true;
bool VPLogger::enable_warning = true;

void SetLogger(bool enable_info, bool enable_warning) {
    VPLogger::enable_info = enable_info;
    VPLogger::enable_warning = enable_warning;
}

VPLogger::VPLogger(bool verbose, const std::string &color, const std::string &prefix) {
    verbose_ = verbose;
    line_ = "";
    prefix_ = prefix;
    if (isatty(fileno(stdout))) {
        color_ = color;
    } else {
        color_ = "";
        color_end_ = "";
    }
}

VPLogger &VPLogger::operator<<(std::ostream &(*os)(std::ostream &)) {
    if (!verbose_) {
        return *this;
    }
    std::cout << prefix_ << " " << line_ << std::endl;
    line_.clear();
    return *this;
}

std::string VPLogger::get_current_time_formatted(const std::string &format) {
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();
    // 转换为 time_t 类型
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    // 将时间格式化为 "YYYY-MM-DD HH:MM:SS"
    std::tm now_tm{};
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&now_tm, &now_c); // Windows平台使用localtime_s
#else
    localtime_r(&now_c, &now_tm); // Linux/Unix平台使用localtime_r
#endif
    std::ostringstream oss;
    oss << std::put_time(&now_tm, format.c_str());
    return oss.str();
}