#include "helpers.h"
#include <cstdio>
#include <cstdlib>
#include <chrono>

void show_warning(const char* const message)
{
    std::fprintf(stderr, "\033[1;33m[!] %s\033[0m\n", message);
}

void show_info(const char* const message)
{
    std::fprintf(stderr, "\033[1;34m[*] %s\033[0m\n", message);
}

void show_error(const char* const message)
{
    std::fprintf(stderr, "\033[1;31m[!] %s\033[0m\n", message);
    std::exit(EXIT_FAILURE);
}
