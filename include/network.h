#ifndef NETWORK_H
#define NETWORK_H
#endif

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

#ifdef _cplusplus
extern "C" {
#endif

void setup_conntrack(const char* const ip_address);
int bizzare_nf_cb(enum nf_conntrack_msg_type type, struct nf_conntrack* ct, void* data);

#ifdef _cplusplus
}
#endif

void network_listen(void);
