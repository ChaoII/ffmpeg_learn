//
// Created by AC on 2024/6/12.
//
#pragma once

#include <iostream>
#include <sstream>
#include "ai_utils.h"

#ifdef _WIN32

#include <io.h>

#define isatty _isatty
#else

#include <unistd.h>

#endif


#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define RESET "\033[0m"


class VPLogger {
public:
    static bool enable_info;
    static bool enable_warning;

    VPLogger() {
        line_ = "";
        prefix_ = "[FastDeploy]";
        verbose_ = true;
    }

    explicit VPLogger(bool verbose, const std::string &color, const std::string &prefix = "[VP]");

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

    std::string get_current_time_formatted(const std::string &format = "%Y-%m-%d %H:%M:%S");

    VPLogger &operator<<(std::ostream &(*os)(std::ostream &));

    ~VPLogger() {
        if (verbose_ && !line_.empty()) {
            std::cout << color_ << get_current_time_formatted() <<" | "<< prefix_ << " | " << line_ << color_end_ << std::endl;
        }
    }


private:
    std::string line_;
    std::string prefix_;
    std::string color_;
    bool verbose_ = true;
    std::string color_end_ = "\033[0m";
};


#ifndef __REL_FILE__
#define __REL_FILE__ __FILE__
#endif

#define VPERROR                                                                \
  VPLogger(true, RED, "[ERROR]")                                                    \
      << __REL_FILE__ << "(" << __LINE__ << ")::" << __FUNCTION__ << "\t"

#define VPWARNING                                                              \
  VPLogger(VPLogger::enable_warning, YELLOW, "[WARNING]")                  \
      << __REL_FILE__ << "(" << __LINE__ << ")::" << __FUNCTION__ << "\t"

#define VPINFO  VPLogger(VPLogger::enable_info, GREEN ,"[INFO]")


