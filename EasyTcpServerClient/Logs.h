#pragma once

#include <iostream>

inline std::mutex gLogMutex;

#define LOG( expr ) \
{\
    std::lock_guard<std::mutex> lock(gLogMutex);\
    std::cout << expr << std::endl; \
}

#define LOG_ERR( expr ) \
{\
    std::lock_guard<std::mutex> lock(gLogMutex);\
    std::cerr << expr << std::endl; \
}
