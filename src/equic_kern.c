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

/**
 * Maximum requests per second allowed from a given source IPv4 address
 */
#define QUIC_RATE_LIMIT 20


/* Map that controls connections counter by source IP */
struct bpf_map_def SEC("maps") counters = {

    .type = BPF_MAP_TYPE_HASH,

    /* A 32 bits representing the source IPv4 address */
    .key_size = sizeof(__be32),

    /* A integer counter of the connections */
    .value_size = sizeof(int),

    .max_entries = 1024,
};


/* Map that controls requests rate limit by source IP */
struct bpf_map_def SEC("maps") rate_limit_map = {

    .type = BPF_MAP_TYPE_HASH,

    /* A 32 bits representing the source IPv4 address */
    .key_size = sizeof(__be32),

    /* A integer counter of the requests */
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
    __u64 begin_in_ns = bpf_ktime_get_ns();

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
        bpf_printk("[XDP] UDP=true, Action=Pass, Status=LookupMiss, Key=%d\n", key);
#endif

            return XDP_PASS;
        }

        /**
         * Start droping traffic from the given IPv4 address once it
         * reached the simultaneous QUIC connections quota
         */
        if (*curr_value >= QUIC_QUOTA_LIMIT) {
#ifdef DEBUG
            bpf_printk("[XDP] Action=Drop, QuotaExceeded=%llu\n", *curr_value);
            __u64 end_in_ns = bpf_ktime_get_ns();
            __u64 elapsed_us = (end_in_ns - begin_in_ns) / 1000;
            bpf_printk(
               "[XDP] BlockDuration=%llu, Begin=%llu, End=%llu\n",
               elapsed_us, begin_in_ns, end_in_ns
            );
#endif
            return XDP_DROP;
        }

#ifdef DEBUG
        bpf_printk("[XDP] Action=Pass, UDP=true, Status=LookupHit, Key=%d\n", key);
#endif
        return XDP_PASS;
    }

#ifdef DEBUG
    bpf_printk("[XDP] Action=Pass, UDP=false\n");
#endif
    return XDP_PASS;
}


char _license[] SEC("license") = "GPL";
