
#include <linux/bpf.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <linux/ip.h>

#include <bpf_helpers.h>


struct bpf_map_def SEC("maps") counters = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(__u32),
    .value_size = sizeof(__u64),
    .max_entries = 512,
};


SEC("drop")
int udp_quota (struct xdp_md *ctx)
{
#ifdef DEBUG
        bpf_printk("[XDP] Action=PacketIn, Packet=%x\n", ctx);
#endif

    __u32 key = 1;
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
#ifdef DEBUG
        bpf_printk("begin(%d) + ip_end(%d) > end(%d)\n", begin, ip_end, end);
#endif
        return XDP_PASS;
    }

    // REMOVE ME
    bpf_printk("exp(%d) == rec(%d)\n", ip->protocol, IPPROTO_UDP);

    if (ip->protocol == IPPROTO_UDP) {

        curr_value = bpf_map_lookup_elem(&counters, &key);
        if(!curr_value) {
#ifdef DEBUG
            bpf_printk("[XDP] Action=Abort Value=%l\n", *curr_value);
#endif
            return XDP_ABORTED;
        }

        if (*curr_value >= 5) {
#ifdef DEBUG
            bpf_printk("[XDP] Action=Drop Value=%l\n", *curr_value);
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
    bpf_printk("[XDP] Action=Drop, UDP=false\n");
#endif
    return XDP_DROP;
}

char _license[] SEC("license") = "GPL";
