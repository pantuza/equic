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


/**
 * SIGINT - Interruption signal callback
 * It unloads eQUIC eBPF program from interface
 */
static void
equic_sigint_callback (int signal)
{
  printf("[eQUIC] Action=exit, Signal=INT\n");

	__u32 prog_id = 0;

	if ( bpf_get_link_xdp_id(EQUIC_IFINDEX, &prog_id, EQUIC_XDP_FLAGS) ) {
		printf("[eQUIC] Error=CallError, Type=BPF, Function=bpf_get_link_xdp_id\n");
		exit(EXIT_FAILURE);
	}

	if ( prog_id == EQUIC_PROG_ID ) {

		bpf_set_link_xdp_fd(EQUIC_IFINDEX, -1, EQUIC_XDP_FLAGS);
		printf("[eQUIC] Action=Unload, Type=BPF, InterfaceIndex=%d\n", EQUIC_IFINDEX);

  } else if (!prog_id) {
		printf("[eQUIC] Error=NotFound, Type=BPF, Message=No program found\n");

  } else {
		printf("[eQUIC] Action=Update, Type=BPF\n");
  }

	exit(EXIT_SUCCESS);
}


/**
 * SIGTERM - Termination signal callback
 * It unloads eQUIC eBPF program from interface
 */
static void
equic_sigterm_callback(int signal)
{
  printf("[eQUIC] Action=exit, Signal=TERM\n");

  /* Meanwhile we just call the same behavior as SIGINT */
  equic_sigint_callback(signal);
}


/**
 * Communicate with Kernel space through eBPF Maps.
 * Syncronize counters map with data from QUIC
 */
static void
equic_sync_counters (int map_fd, int interval)
{
  while (true) {

    printf("[eQUIC] Action=Sync, Type=BPF, Map=counters\n");
    sleep(interval);
  }
}


/**
 * Reads the network interface index from
 * a given name
 */
static void
equic_get_interface (equic_t *equic, char if_name[])
{
	equic->if_index = if_nametoindex(if_name);
  EQUIC_IFINDEX = equic->if_index;

	if ( !equic->if_index ) {
    printf("[eQUIC] Error=Name to index failed, Type=OS, Interface=%s\n", if_name);
		exit(EXIT_FAILURE);
	}

  printf("[eQUIC] Action=Read, Type=OS, Interface=%d\n", equic->if_index);
}

/**
 * Reads the .o object program to be loaded in the
 * kernel. This functions reads all maps and the
 * kernel hook function
 */
static void
equic_read_object (equic_t *equic, char bpf_prog_path[])
{
  struct bpf_prog_load_attr prog_attr = {
    .prog_type	= BPF_PROG_TYPE_XDP,
    .file       = bpf_prog_path,
  };
	struct bpf_object *obj;

	if ( bpf_prog_load_xattr(&prog_attr, &obj, &equic->prog_fd) ) {
    printf("[eQUIC] Error=Load prog attr falied, Type=BPF\n");
		exit(EXIT_FAILURE);
  }
  printf("[eQUIC] Action=Load, Type=BPF, Object=%s\n", bpf_prog_path);

	struct bpf_map *map;
	map = bpf_map__next(NULL, obj);
	if ( !map ) {
    printf("[eQUIC] Error=Find map failed, Type=BPF\n");
		exit(EXIT_FAILURE);
	}
  equic->counters_map_fd = bpf_map__fd(map);
  printf("[eQUIC] Action=Load, Type=BPF, Map=counters\n");

	if ( !equic->prog_fd ) {
    printf("[eQUIC] Error=%s, Type=BPF, Function=bpf_prog_load_xattr\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}


/**
 * Loads the eBPF kernel program on the linux kernel
 * XDP hook on the network interface
 */
static void
equic_load (equic_t *equic)
{
	if (bpf_set_link_xdp_fd(equic->if_index, equic->prog_fd, EQUIC_XDP_FLAGS) < 0) {
    printf("[eQUIC] Error=Link setup failed, Type=BPF, Function=bpf_set_link_xdp_fd\n");
		exit(EXIT_FAILURE);
	}

	struct bpf_prog_info info = {};
	__u32 info_len = sizeof(info);

	if (bpf_obj_get_info_by_fd(equic->prog_fd, &info, &info_len)) {
    printf("[eQUIC] Error=%s, Type=BPF, Function=bpf_obj_get_info_by_fd\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	EQUIC_PROG_ID = info.id;

  printf("[eQUIC] Action=Setup, Type=BPF, Hook=XDP\n");
}

int
main(int argc, char **argv)
{
  equic_t equic;

  equic_get_interface(&equic, "eth0");
  equic_read_object(&equic, "/src/equic/bin/equic_kern.o");

  /* Sets signal handlers */
  signal(SIGINT, equic_sigint_callback);
  signal(SIGTERM, equic_sigterm_callback);

  equic_load(&equic);
  equic_sync_counters(equic.counters_map_fd, 3);

  return EXIT_SUCCESS;
}
