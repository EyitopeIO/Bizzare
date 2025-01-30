#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <string>
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

    Args args(argc, argv);

    if (args.debug_mode) {
        show_info("Debug mode enabled");
    }

    if (args.ip_address.empty()) {
        show_error("IP address not provided");
    }

    setup_conntrack();

    return 0;
}