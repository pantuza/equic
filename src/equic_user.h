#ifndef EQUIC_H
#define EQUIC_H


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



/* Network interface index */
static int EQUIC_IFINDEX;

/* eBPF program ID */
static __u32 EQUIC_PROG_ID;

/* XDP Flags used for eQUIC program */
static unsigned int EQUIC_XDP_FLAGS = XDP_FLAGS_UPDATE_IF_NOEXIST;


/**
 * SIGINT - Interruption signal callback
 * It unloads eQUIC eBPF program from interface
 */
static void sigint_callback(int signal)
{
  printf("[eQUIC] Action=exit, Signal=INT");

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
static void sigterm_callback(int signal)
{
  printf("[eQUIC] Action=exit, Signal=TERM");

  /* Meanwhile we just call the same behavior as SIGINT */
  sigint_callback(signal);
}


/* simple per-protocol drop counter
 */
static void poll_stats(int map_fd, int interval)
{
	while (1) {
		sleep(interval);
		printf("Reading counters map\n");
	}
}

#endif /* EQUIC_H */
