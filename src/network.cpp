#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include "network.h"
#include "helpers.h"


struct nfct_handle *g_handle;

extern "C" int bizzare_nf_cb(enum nf_conntrack_msg_type* type, struct nf_conntrack* ct, void* data) {
    return NFCT_CB_CONTINUE;
}


void setup_conntrack(void) {

    g_handle = nfct_open(CONNTRACK, 0);
    if (!g_handle) {
        show_error("Failed to open nfct handle");
    }

}