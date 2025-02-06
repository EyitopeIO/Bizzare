#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <thread>
#include <csignal>
#include "helpers.h"
#include "network.h"

static void show_help(void)
{
    std::cout << "Usage: bizzare [OPTION]... [FILE]..." << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -d      Enable debug mode" << std::endl;
    std::cout << "  -h      Display this help and exit" << std::endl;
    std::cout << "  -m      Device MAC address" << std::endl;
    std::exit(EXIT_FAILURE);
}

extern struct nfct_handle *g_handle;

static void sigint_handler(int sig) {
    show_info("Shutting down");
    nfct_close(g_handle);
    std::exit(EXIT_SUCCESS);
}

std::chrono::time_point<std::chrono::steady_clock> g_bizzare_launch_time;

class Args {

    public:

        std::string ip_address;
        bool debug_mode;

        Args(int argc, char* argv[]) {
            int opt;
            while ((opt = getopt(argc, argv, "a:dh")) != -1) {
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

                    default:
                        break;
                }
            }

            if (optind > argc) {
                show_warning("Ignoring extra arguments");
            }
        }
};


int main(int argc, char* argv[]) {

    g_bizzare_launch_time = std::chrono::steady_clock::now();
    std::setbuf(stdout, nullptr);

    Args args(argc, argv);

    if (args.debug_mode) {
        show_info("Debug mode enabled");
    }

    if (args.ip_address.empty()) {
        show_error("IP address not provided");
    }

    setup_conntrack(args.ip_address.c_str());
    show_info_cpp("Listening to " << args.ip_address << " ...");
    std::thread network_thread(network_listen);

    std::signal(SIGINT, sigint_handler);

    network_thread.join();

    return 0;
}