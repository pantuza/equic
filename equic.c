
#include <linux/bpf.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <linux/ip.h>

#include <bpf_helpers.h>


#define UDP_QUOTA_LIMIT 5


struct bpf_map_def SEC("maps") counters = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(__u32),
    .value_size = sizeof(__u64),
    .max_entries = 512,
};


SEC("equic")
int udp_quota (struct xdp_md *ctx)
{
    __u32 key = 0;
    __u64 value = 0;
    __u64 *curr_value;

    bpf_map_update_elem(&counters, &key, &value, BPF_NOEXIST);

    void *begin = (void *)(long)ctx->data;
    void *end = (void *)(long)ctx->data_end;

    struct ethhdr *eth = begin;

    int ip_begin = sizeof(*eth);
    struct iphdr *ip = begin + ip_begin;
    int ip_end = ip_begin + sizeof(struct iphdr);

    if (begin + ip_end > end) {
        return XDP_ABORTED;
    }

    if (ip->protocol == IPPROTO_UDP) {

        curr_value = bpf_map_lookup_elem(&counters, &key);
        if(!curr_value) {
            return XDP_ABORTED;
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
