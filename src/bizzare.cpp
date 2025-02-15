#include <iostream>
#include <cstdio>
#include <string>
#include <thread>
#include <csignal>
#include "helpers.h"
#include "network.h"
#include "database.h"


extern struct nfct_handle *g_handle;
extern bool g_network_loop_conitnue;

static void sigint_handler(int sig) {
    show_info("Shutting down");
    nfct_close(g_handle);
    g_network_loop_conitnue = false;
}

std::chrono::time_point<std::chrono::steady_clock> g_bizzare_launch_time;
Args g_args;


int main(int argc, char* argv[]) {

    g_bizzare_launch_time = std::chrono::steady_clock::now();
    std::setbuf(stdout, nullptr);

    g_args.parse(argc, argv);

    if (g_args.debug_mode) {
        show_info("Debug mode enabled");
    }

    if (g_args.ip_address.empty()) {
        show_error("IP address not provided");
    }

    if (g_args.nat_ip_address.empty()) {
        g_args.nat_ip_address = g_args.ip_address;
        show_info("No NAT IP address provided");
    }

    if (!g_args.poll_interval) {
        g_args.poll_interval = BIZZARE_NF_QUERRY_INTERVAL_MS;
    } else if (g_args.poll_interval > BIZZARE_NF_QUERRY_INTERVAL_MAX) {
        g_args.poll_interval = BIZZARE_NF_QUERRY_INTERVAL_MAX;
        show_warning_cpp("Polling interval capped at " << BIZZARE_NF_QUERRY_INTERVAL_MAX << " milliseconds");
    }

    db_setup();

    setup_conntrack(g_args.ip_address.c_str());
    show_info_cpp("Listening to " << g_args.ip_address << " ...");
    uint32_t pref = 2;
    std::thread network_thread(network_listen);

    std::signal(SIGINT, sigint_handler);

    network_thread.join();

    return 0;
}