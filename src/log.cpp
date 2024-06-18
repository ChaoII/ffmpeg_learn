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