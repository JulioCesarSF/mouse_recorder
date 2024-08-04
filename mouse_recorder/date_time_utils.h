#pragma once

#include <sstream>
#include <iostream>
#include <chrono>
#define __STDC_WANT_LIB_EXT1__ 1
#define _XOPEN_SOURCE // for putenv
#include <stdio.h>
#include <stdlib.h>   // for putenv
#include <time.h>

namespace date_time_utils
{
    std::string get_datetime_now() 
    {
        std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::string s(30, '\0');
        struct tm buf;
        localtime_s(&buf, &now);
        std::strftime(&s[0], s.size(), "%Y-%m-%d %H:%M:%S", &buf);
        return s;
    }

    std::string format_duration(std::chrono::milliseconds ms) 
    {
        using namespace std::chrono;
        auto secs = duration_cast<seconds>(ms);
        ms -= duration_cast<milliseconds>(secs);
        auto mins = duration_cast<minutes>(secs);
        secs -= duration_cast<seconds>(mins);
        auto hour = duration_cast<hours>(mins);
        mins -= duration_cast<minutes>(hour);

        std::stringstream ss;
        ss << hour.count() << " Hours : " << mins.count() << " Minutes : " << secs.count() << " Seconds : " << ms.count() << " Milliseconds";
        return ss.str();
    }
}