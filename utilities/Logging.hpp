#include <iostream>
#include <sstream>

#ifndef CPP_MODULE
#define CPP_MODULE "UNKN"
#endif

// Basic logging message
#define LOG(msg) std::cout << CPP_MODULE << ": " << msg << "\n";

// Logging with error, throws an exception as well
#define LOG_ERROR(msg) {\
    std::stringstream sErrorMessage;\
    sErrorMessage << CPP_MODULE << " ERROR(" << __LINE__<< "): " <<  msg << "\n"; \
    std::cerr << sErrorMessage.str(); \
    throw std::runtime_error(sErrorMessage.str()); }\

#define LOG_WARNING(msg) {\
    std::stringstream sErrorMessage;\
    sErrorMessage << CPP_MODULE << " WARNING(" << __LINE__<< "): " <<  msg << "\n"; \
}\

// Assertion check
#define LOG_ASSERT(cond, msg)\
    do { \
        if (!(cond)) { \
            std::cerr << CPP_MODULE << " ERROR(" << __LINE__<< "): " <<  msg << "\n"; \
            exit(EXIT_FAILURE); \
        } \
    } while(0) \

