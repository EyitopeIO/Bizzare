#include "helpers.h"
#include <cstdio>
#include <unistd.h>
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
    std::perror(nullptr);
    std::exit(EXIT_FAILURE);
}


static void show_help(void)
{
    std::cout << "Usage: bizzare [OPTION]... [FILE]..." << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -d      Enable debug mode" << std::endl;
    std::cout << "  -h      Display this help and exit" << std::endl;
    std::cout << "  -a      Device IP address" << std::endl;
    std::cout << "  -b      Next hop NAT-ed address" << std::endl;
    std::cout << "  -t      Polling interval in milliseconds" << std::endl;
}

Args::Args(void) {
    poll_interval = 0;
    debug_mode = false;
}

void Args::parse(int argc, char* argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "a:t:b:dh")) != -1) {
        switch(opt) {
            case 'd':
                debug_mode = true;
                break;

            case 'h':
                show_help();
                std::exit(EXIT_SUCCESS);
                break;

            case 'a':
                ip_address = optarg;
                break;

            case 'b':
                nat_ip_address = optarg;
                break;

            case 't':
                poll_interval = std::stoi(optarg);
                break;

            default:
                show_help();
                std::exit(EXIT_FAILURE);
                break;
        }
    }

    if (optind > argc) {
        show_warning("Ignoring extra arguments");
    }
}
