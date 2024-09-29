#pragma once

#include <iostream>

inline std::mutex gLogMutex;

#ifndef LOG
    #define LOG( expr ) \
    {\
        std::lock_guard<std::mutex> lock(gLogMutex);\
        std::cout << expr << std::endl; \
    }
#endif

#ifndef LOG_ERR
    #define LOG_ERR( expr ) \
    {\
        std::lock_guard<std::mutex> lock(gLogMutex);\
        std::cerr << __FILE__ << ": " << __LINE__ << std::endl; \
        std::cerr << expr << std::endl; \
    }
#endif
