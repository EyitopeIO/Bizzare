
#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <iostream>
#include <ctime>
#include <chrono>

#ifdef _cplusplus
extern "C" {
#endif

void show_warning(const char* const message);
void show_info(const char* const message);
void show_error(const char* const message);


#ifdef _cplusplus
}
#endif

extern std::chrono::time_point<std::chrono::steady_clock> g_bizzare_launch_time;
#define time_now_ms [](void) { \
    auto e_t = std::chrono::steady_clock::now() - g_bizzare_launch_time; \
    return std::chrono::duration_cast<std::chrono::milliseconds>(e_t).count(); \
}()

#define time_now_hhmmss [](void) { \
    std::time_t now = std::time(nullptr); \
    std::tm local_time = *std::localtime(&now); \
    std::string ret = ""; \
    ret = std::string(local_time.tm_hour < 10 ? "0" : "") + std::to_string(local_time.tm_hour) + std::string(":") \
        + std::string(local_time.tm_min < 10 ? "0" : "") + std::to_string(local_time.tm_min) + std::string(":") \
        + std::string(local_time.tm_sec < 10 ? "0" : "") + std::to_string(local_time.tm_sec); \
    return ret; \
}()

#define show_warning_cpp(message) do \
{ \
    std::cerr << "\033[1;33m[" << time_now_ms << "]" << " " << message << "\033[0m\n"; \
} while(0);

#define show_info_cpp(message) do \
{ \
    std::cerr << "\033[1;34m[" << time_now_ms << "]" << " " <<  message << "\033[0m\n"; \
} while(0);

#define show_error_cpp(message) do \
{ \
    std::cerr << "\033[1;31m[" << time_now_ms << "]" << " " << message << "\033[0m\n"; \
    std::exit(EXIT_FAILURE); \
} while(0);


class Args {
    public:
        std::string ip_address;
        uint32_t poll_interval;
        bool debug_mode;
        Args(void);
        void parse(int argc, char* argv[]);
};

extern Args g_args;

#endif