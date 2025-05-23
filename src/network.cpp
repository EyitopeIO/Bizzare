#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include "network.h"
#include "helpers.h"
#include "database.h"
#include <thread>
#include <fstream>

#define BIZZARE_NF_ACCT_VARIABLE "/proc/sys/net/netfilter/nf_conntrack_acct"

struct nfct_handle *g_handle;
bool g_network_loop_conitnue = true;

uint64_t g_tx_bytes = 0;
uint64_t g_rx_bytes = 0;

static std::chrono::time_point<std::chrono::high_resolution_clock> g_request_time;
static std::chrono::time_point<std::chrono::high_resolution_clock> g_response_time;


static void show_event(enum nf_conntrack_msg_type type, struct nf_conntrack* ct) {
    char buf[1024];
    nfct_snprintf(buf, sizeof(buf), ct, type, NFCT_O_PLAIN, NFCT_OF_TIME);
    std::printf("%s\n", buf);
}

int bizzare_nf_cb(enum nf_conntrack_msg_type type, struct nf_conntrack* ct, void* data) {

    uint8_t proto = nfct_get_attr_u8(ct,  ATTR_ORIG_L3PROTO);
    if (type == NFCT_T_ERROR) {
        show_error("Error in conntrack");
        return NFCT_CB_STOP;
    }

    switch(proto) {
        case AF_INET:
            break;
        case AF_INET6:
            show_warning_cpp("Ignoring IPv6");
            break;
        default:
            show_warning_cpp("Unsupported protocol");
            return NFCT_CB_CONTINUE;
    }

    static uint32_t ip = htonl(inet_addr(g_args.ip_address.c_str()));
    static uint32_t nat_ip = htonl(inet_addr(g_args.nat_ip_address.c_str()));

    uint32_t orig = htonl(nfct_get_attr_u32(ct, ATTR_ORIG_IPV4_SRC));
    uint32_t repl = htonl(nfct_get_attr_u32(ct, ATTR_REPL_IPV4_DST));
    if (orig != ip && repl != nat_ip) {
        return NFCT_CB_CONTINUE;
    }

    uint64_t tx = nfct_get_attr_u64(ct, ATTR_ORIG_COUNTER_BYTES);
    uint64_t rx = nfct_get_attr_u64(ct, ATTR_REPL_COUNTER_BYTES);
    g_tx_bytes += tx;
    g_rx_bytes += rx;

    if (g_args.debug_mode) {
        show_event(type, ct);
        show_info_cpp("tx " << tx);
        show_info_cpp("rx " << rx);
    }

    return NFCT_CB_CONTINUE;
}

void setup_conntrack(const char* const ip_address) {

    std::fstream nf_acct_var(BIZZARE_NF_ACCT_VARIABLE);
    if (nf_acct_var.is_open()) {
        if (nf_acct_var.get() == '0') {
            show_info_cpp("Enabled conntrack accounting");
            nf_acct_var << "1";
        }
    } else {
        show_warning_cpp("Can't open " << BIZZARE_NF_ACCT_VARIABLE);
    }
    nf_acct_var.close();

    g_handle = nfct_open(CONNTRACK, NFCT_ALL_CT_GROUPS);
    if (!g_handle) {
        show_error("Failed to open nfct handle");
    }
	nfct_callback_register(g_handle, NFCT_T_ALL, bizzare_nf_cb, nullptr);
}


void network_listen(void) {
    int ret = 0;
    uint32_t total_querries = 0;
    uint32_t family = AF_INET;

    while (g_network_loop_conitnue) {
        g_tx_bytes = 0;
        g_rx_bytes = 0;

        g_request_time = std::chrono::high_resolution_clock::now();
        ret = nfct_query(g_handle, NFCT_Q_DUMP, &family);
        if (ret == -1) {
            perror("nfct_query");
        }
        g_response_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(g_response_time - g_request_time).count();

        total_querries++;

        if (g_tx_bytes || g_rx_bytes) {
            db_save( unix_time_stamp, duration,
                g_rx_bytes / (1024.0 * 1024.0),
                g_tx_bytes / (1024.0 * 1024.0)
            );
        }

        if (g_args.debug_mode) {
            show_info_cpp( \
                std::endl \
                << "========" << std::endl \
                << "querry " << total_querries << std::endl \
                << "duration: " << duration << " ms" << std::endl \
                << "ttx: " << g_tx_bytes << std::endl \
                << "trx: " << g_rx_bytes << std::endl \
                << "========" << std::endl);
        }

        if (duration < g_args.poll_interval) {
            std::this_thread::sleep_for(std::chrono::milliseconds(g_args.poll_interval - duration));
        }
    }
}

int get_ipv4_address(std::string& ip_address) {
    struct ifaddrs *cursor, *p_iface_addr = nullptr;

    getifaddrs(&p_iface_addr);
    if (p_iface_addr == nullptr) {
        show_error("getifaddrs");
        return -1;
    }

    for (cursor = p_iface_addr; cursor != nullptr; cursor = cursor->ifa_next) {
        if (cursor->ifa_addr == nullptr) {
            continue;
        }
        if (cursor->ifa_addr->sa_family != AF_INET) {
            continue;
        }
        if (!std::strcmp(cursor->ifa_name, "br-lan") ||
            !std::strcmp(cursor->ifa_name, "lan") ||
            !std::strcmp(cursor->ifa_name, "br0") ||
            !std::strcmp(cursor->ifa_name, "br-wlan")) {
                ip_address = std::string(inet_ntoa(((struct sockaddr_in *)cursor->ifa_addr)->sin_addr));
                break;
        } else {
            if (g_args.debug_mode) {
                show_info_cpp("Ignoring interface " << cursor->ifa_name);
            }
        }
    }

    freeifaddrs(p_iface_addr);

    if (ip_address.empty()) {
        return -1;
    }
    return 0;
}

