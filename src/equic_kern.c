
#include <linux/bpf.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>

#include "bpf/bpf_helpers.h"
#include "bpf/bpf_endian.h"


#define UDP_QUOTA_LIMIT 10


struct map_key {
    __be32 s_addr;
    __be16 s_port;

    __be32 d_addr;
    __be16 d_port;
};

typedef struct map_key map_key_t;


struct bpf_map_def SEC("maps") counters = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(map_key_t),
    .value_size = sizeof(__u64),
    .max_entries = 1024,
};


SEC("equic")
int udp_quota (struct xdp_md *ctx)
{
    __u64 value = 0;
    __u64 *curr_value;

    map_key_t key;
    __builtin_memset(&key, 0, sizeof(map_key_t));

    // bpf_map_update_elem(&counters, &key, &value, BPF_NOEXIST);

    void *begin = (void *)(long)ctx->data;
    void *end = (void *)(long)ctx->data_end;

    struct ethhdr *eth = begin;

    int ip_begin = sizeof(*eth);
    struct iphdr *ip = begin + ip_begin;
    int ip_end = ip_begin + sizeof(struct iphdr);

    if (begin + ip_end > end) {
        return XDP_ABORTED;
    }

    key.s_addr = ip->saddr;
    key.d_addr = ip->daddr;

    if (ip->protocol == IPPROTO_UDP) {

        struct udphdr *udp = (void*)ip + sizeof(*ip);
        if ((void *)udp + sizeof(*udp) > end) {
            return XDP_ABORTED;
        }

        key.s_port = udp->source;
        key.d_port = udp->dest;

        curr_value = bpf_map_lookup_elem(&counters, &key);
        if(!curr_value) {
#ifdef DEBUG
        bpf_printk("[XDP] UDP=true, Action=Pass, Status=LookupMiss Key=(%d,%d)\n",
                    bpf_ntohl(key.s_port), bpf_ntohl(key.d_port));
//                   inet_ntoa(key.s_addr), ntohl(key.s_port),
//                   inet_ntoa(key.d_addr), ntohl(key.d_port));
#endif

            return XDP_PASS;
        }

        if (*curr_value >= UDP_QUOTA_LIMIT) {
#ifdef DEBUG
            bpf_printk("[XDP] Action=Drop QuotaExceeded=%llu\n", *curr_value);
#endif
            return XDP_DROP;
        }

        *curr_value += 1;
        bpf_map_update_elem(&counters, &key, curr_value, BPF_EXIST);

#ifdef DEBUG
        bpf_printk("[XDP] Action=Pass, UDP=true\n");
#endif
        return XDP_PASS;
    }


#ifdef DEBUG
    bpf_printk("[XDP] Action=Pass, UDP=false\n");
#endif
    return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
