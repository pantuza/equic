
#include <linux/bpf.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>

#include "bpf/bpf_helpers.h"
#include "bpf/bpf_endian.h"


#define UDP_QUOTA_LIMIT 5



struct bpf_map_def SEC("maps") counters = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(__be32),
    .value_size = sizeof(int),
    .max_entries = 1024,
};


SEC("equic")
int udp_quota (struct xdp_md *ctx)
{
    __be32 key;
    int value = 0;

    void *begin = (void *)(long)ctx->data;
    void *end = (void *)(long)ctx->data_end;

    struct ethhdr *eth = begin;

    int ip_begin = sizeof(*eth);
    struct iphdr *ip = begin + ip_begin;
    int ip_end = ip_begin + sizeof(struct iphdr);

    if (begin + ip_end > end) {
        return XDP_ABORTED;
    }

    key = ip->saddr;

    if (ip->protocol == IPPROTO_UDP) {

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

        if (*curr_value >= UDP_QUOTA_LIMIT) {
#ifdef DEBUG
            bpf_printk("[XDP] Action=Drop QuotaExceeded=%llu\n", *curr_value);
#endif
            return XDP_DROP;
        }

#ifdef DEBUG
        bpf_printk("[XDP] Action=Pass, UDP=true, Status=LookupHit Key=%s\n", key);
#endif
        return XDP_PASS;
    }


#ifdef DEBUG
    bpf_printk("[XDP] Action=Pass, UDP=false\n");
#endif
    return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
