#include <string>
#include <arpa/inet.h>
#include "network.h"
#include "helpers.h"
#include <thread>
#include <fstream>


#define NF_ACCT_VARIABLE "/proc/sys/net/netfilter/nf_conntrack_acct"

struct nfct_handle *g_handle;
struct nfct_filter *g_filter;

uint64_t g_tx_bytes = 0;
uint64_t g_rx_bytes = 0;

int bizzare_nf_cb(enum nf_conntrack_msg_type type, struct nf_conntrack* ct, void* data) {

#if DEBUG
    char buf[1024];
    nfct_snprintf(buf, sizeof(buf), ct, type, NFCT_O_PLAIN, NFCT_OF_TIME);
    std::printf("%s\n", buf);
#endif

    uint8_t proto = nfct_get_attr_u8(ct,  ATTR_ORIG_L3PROTO);
    if (type == NFCT_T_DESTROY || type == NFCT_T_ERROR) {
        return NFCT_CB_CONTINUE;
    }

    switch(proto) {
        case AF_INET:
        case AF_INET6:
            break;
        default:
            show_warning_cpp("Unsupported protocol");
            return NFCT_CB_CONTINUE;
    }

    uint64_t tx = nfct_get_attr_u64(ct, ATTR_ORIG_COUNTER_BYTES);
    uint64_t rx = nfct_get_attr_u64(ct, ATTR_REPL_COUNTER_BYTES);

    if (rx) {
        g_rx_bytes = rx;
    } else {
        show_warning_cpp("0 rx bytes received");
        if (nfct_attr_is_set(ct,ATTR_ORIG_COUNTER_BYTES) == -1) {
            std::perror(nullptr);
        }
    }

    if (tx) {
        g_tx_bytes = tx;
    } else {
        show_warning_cpp("0 tx bytes received");
        if (nfct_attr_is_set(ct, ATTR_ORIG_COUNTER_BYTES) == -1) {
            std::perror(nullptr);
        }
    }

    return NFCT_CB_CONTINUE;
}

void setup_conntrack(const char* const ip_address) {

    std::fstream nf_acct_var(NF_ACCT_VARIABLE);
    if (nf_acct_var.is_open()) {
        if (nf_acct_var.get() == '0') {
            show_info_cpp("Enabled conntrack accounting");
            nf_acct_var << "1";
        }
    } else {
        show_warning_cpp("Can't open " << NF_ACCT_VARIABLE);
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
    nfct_catch(g_handle);
}

