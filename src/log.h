//
// Created by AC on 2024/6/12.
//
#pragma once

#include <iostream>
#include <sstream>


class VPLogger {
public:
    static bool enable_info;
    static bool enable_warning;

    VPLogger() {
        line_ = "";
        prefix_ = "[FastDeploy]";
        verbose_ = true;
    }

    explicit VPLogger(bool verbose, const std::string &prefix = "[FastDeploy]");

    template<typename T>
    VPLogger &operator<<(const T &val) {
        if (!verbose_) {
            return *this;
        }
        std::stringstream ss;
        ss << val;
        line_ += ss.str();
        return *this;
    }

    VPLogger &operator<<(std::ostream &(*os)(std::ostream &));

    ~VPLogger() {
        if (verbose_ && !line_.empty()) {
            std::cout << prefix_ << " " << line_ << std::endl;
        }
    }

private:
    std::string line_;
    std::string prefix_;
    bool verbose_ = true;
};


#ifndef __REL_FILE__
#define __REL_FILE__ __FILE__
#endif

#define VPERROR                                                                \
  VPLogger(true, "[ERROR]")                                                    \
      << __REL_FILE__ << "(" << __LINE__ << ")::" << __FUNCTION__ << "\t"

#define VPWARNING                                                              \
  VPLogger(VPLogger::enable_warning, "[WARNING]")                  \
      << __REL_FILE__ << "(" << __LINE__ << ")::" << __FUNCTION__ << "\t"

#define VPINFO  VPLogger(VPLogger::enable_info, "[INFO]")<< "\t"


