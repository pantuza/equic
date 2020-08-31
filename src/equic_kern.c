/* Linux headers for bpf and networking */
#include <linux/bpf.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>

/* libbpf header files */
#include "bpf/bpf_helpers.h"
#include "bpf/bpf_endian.h"


/**
 * Maximum simultaneous connections allowed from a given source IPv4 address
 */
#define QUIC_QUOTA_LIMIT 5



/* Map that controls connections counter by source IP */
struct bpf_map_def SEC("maps") counters = {

    .type = BPF_MAP_TYPE_HASH,

    /* A 32 bits representing the source IPv4 address */
    .key_size = sizeof(__be32),

    /* A integer counter of the connections */
    .value_size = sizeof(int),

    .max_entries = 1024,
};



/**
 * eQUIC XDP hook that verifies if a given source IPv4 address
 * has reached its connection quota limit
 */
SEC("equic")
int udp_quota (struct xdp_md *ctx)
{
    __be32 key;
    int value = 0;

    void *begin = (void *)(long)ctx->data;
    void *end = (void *)(long)ctx->data_end;

    /* Assign a Ethernet header */
    struct ethhdr *eth = begin;

    /* Assign a IPv4 header */
    int ip_begin = sizeof(*eth);
    struct iphdr *ip = begin + ip_begin;
    int ip_end = ip_begin + sizeof(struct iphdr);

    if (begin + ip_end > end) {
        return XDP_ABORTED;
    }

    /* Reads the packet source IPv4 address used as map key */
    key = ip->saddr;

    if (ip->protocol == IPPROTO_UDP) {

        /* Assigns a UDP header */
        struct udphdr *udp = (void*)ip + sizeof(*ip);
        if ((void *)udp + sizeof(*udp) > end) {
            return XDP_ABORTED;
        }

        int *curr_value = bpf_map_lookup_elem(&counters, &key);
        if(!curr_value) {
#ifdef DEBUG
        bpf_printk("[XDP] UDP=true, Action=Pass, Status=LookupMiss Key=%d\n", key);
#endif

            return XDP_PASS;
        }

        /**
         * Start droping traffic from the given IPv4 address once it
         * reached the simultaneous QUIC connections quota
         */
        if (*curr_value >= QUIC_QUOTA_LIMIT) {
#ifdef DEBUG
            bpf_printk("[XDP] Action=Drop QuotaExceeded=%llu\n", *curr_value);
#endif
            return XDP_DROP;
        }

#ifdef DEBUG
        bpf_printk("[XDP] Action=Pass, UDP=true, Status=LookupHit Key=%d\n", key);
#endif
        return XDP_PASS;
    }

#ifdef DEBUG
    bpf_printk("[XDP] Action=Pass, UDP=false\n");
#endif
    return XDP_PASS;
}


char _license[] SEC("license") = "GPL";
