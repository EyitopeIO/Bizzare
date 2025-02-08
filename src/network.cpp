#include <string>
#include <arpa/inet.h>
#include "network.h"
#include "helpers.h"
#include <thread>
#include <fstream>


#define BIZZARE_NF_ACCT_VARIABLE "/proc/sys/net/netfilter/nf_conntrack_acct"

struct nfct_handle *g_handle;
struct nfct_filter *g_filter;
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

    uint64_t tx = nfct_get_attr_u64(ct, ATTR_ORIG_COUNTER_BYTES);
    uint32_t orig = htonl(nfct_get_attr_u32(ct, ATTR_ORIG_IPV4_SRC));
    if (orig == ip) {
        show_event(type, ct);
        g_tx_bytes += tx;
        if (g_args.debug_mode) {
            show_info_cpp("tx " << tx);
        }
    }

    uint64_t rx = nfct_get_attr_u64(ct, ATTR_REPL_COUNTER_BYTES);
    uint32_t repl = htonl(nfct_get_attr_u32(ct, ATTR_REPL_IPV4_DST));
    if (repl == ip) {
        show_event(type, ct);
        g_rx_bytes += rx;
        if (g_args.debug_mode) {
            show_info_cpp("rx " << rx);
        }
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

    g_handle = nfct_open(NFNL_SUBSYS_CTNETLINK, NFCT_ALL_CT_GROUPS);
    if (!g_handle) {
        show_error("Failed to open nfct handle");
    }

    g_filter = nfct_filter_create();
    if (!g_filter) {
        show_error("Failed to create nfct filter");
    }

    struct nfct_filter_ipv4 filter_ipv4 = {
		.addr = ntohl(inet_addr(ip_address)),
		.mask = 0xFFFFFFFF,
	};

    if (nfct_filter_set_logic(g_filter, NFCT_FILTER_SRC_IPV4, NFCT_FILTER_LOGIC_POSITIVE) == -1) {
        show_error("Failed to set src filter logic");
    }

    if (nfct_filter_set_logic(g_filter, NFCT_FILTER_DST_IPV4, NFCT_FILTER_LOGIC_POSITIVE) == -1) {
        show_error("Failed to set dest filter logic");
    }
    
	nfct_filter_add_attr(g_filter, NFCT_FILTER_SRC_IPV4, &filter_ipv4);

	if (nfct_filter_attach(nfct_fd(g_handle), g_filter) == -1) {
		show_error("Failed to attach nfct filter");
	}

	nfct_filter_destroy(g_filter);

	nfct_callback_register(g_handle, NFCT_T_ALL, bizzare_nf_cb, nullptr);

    show_info("Conntrack setup complete");
}


void network_listen(void) {
    int ret = 0;
    uint32_t total_querries = 0;
    uint32_t family = AF_INET;

    while (g_network_loop_conitnue) {
        g_tx_bytes = 0;
        g_rx_bytes = 0;

        g_request_time = std::chrono::high_resolution_clock::now();
        ret = nfct_query(g_handle, NFCT_Q_DUMP_FILTER, &family);
        if (ret == -1) {
            show_warning_cpp("Failed to query conntrack");
        }
        g_response_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(g_response_time - g_request_time).count();

        total_querries++;

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

