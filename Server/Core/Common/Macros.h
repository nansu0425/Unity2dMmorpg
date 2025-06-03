/*    Core/Common/Macro.h    */

#pragma once

#ifdef _DEBUG
#define CORE_ASSERT(condition, message) \
        do { \
            if (!(condition)) { \
                std::cerr << "Assertion failed: " << #condition << "\n" \
                          << "File: " << __FILE__ << "\n" \
                          << "Line: " << __LINE__ << "\n" \
                          << "Message: " << message << std::endl; \
                assert(false); \
            } \
        } while (false)
#else
#define CORE_ASSERT(condition, message) do { } while (false)
#endif
