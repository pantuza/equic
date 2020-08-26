#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <net/if.h>

/* Linux OS dependent */
#include <sys/resource.h>
#include <linux/bpf.h>
#include <linux/if_link.h>


/* libbpf included headers */
#include <bpf/bpf.h>
#include <bpf/libbpf.h>

#include "equic_user.h"


int
main(int argc, char **argv)
{
	struct bpf_prog_load_attr prog_load_attr = {
		.prog_type	= BPF_PROG_TYPE_XDP,
        .file       = "equic.o",
	};

	struct bpf_prog_info info = {};
	__u32 info_len = sizeof(info);
	const char *optstr = "FSN";
	int prog_fd, map_fd, opt;
	struct bpf_object *obj;
	struct bpf_map *map;
	int err;


	EQUIC_IFINDEX = if_nametoindex("eth0");
	if (!EQUIC_IFINDEX) {
		perror("if_nametoindex");
		return 1;
	}

	if (bpf_prog_load_xattr(&prog_load_attr, &obj, &prog_fd)) {
		printf("Failed to load eBPF program attributes\n");
		return 1;
    }

	map = bpf_map__next(NULL, obj);
	if (!map) {
		printf("finding a map in obj file failed\n");
		return 1;
	}
	map_fd = bpf_map__fd(map);

	if (!prog_fd) {
		printf("bpf_prog_load_xattr: %s\n", strerror(errno));
		return 1;
	}

	signal(SIGINT, sigint_callback);
	signal(SIGTERM, sigterm_callback);

	if (bpf_set_link_xdp_fd(EQUIC_IFINDEX, prog_fd, EQUIC_XDP_FLAGS) < 0) {
		printf("link set xdp fd failed\n");
		return 1;
	}

	err = bpf_obj_get_info_by_fd(prog_fd, &info, &info_len);
	if (err) {
		printf("can't get prog info - %s\n", strerror(errno));
		return err;
	}
	EQUIC_PROG_ID = info.id;


	poll_stats(map_fd, 2);

	return EXIT_SUCCESS;
}
