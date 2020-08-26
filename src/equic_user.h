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

/* Struct used to store eQUIC public variables */
struct equic_meta
{
  /* Interface Index */
  int if_index;

  /* eBPF program file descriptor */
  int prog_fd;

  /* Counters Map file descriptor */
  int counters_map_fd;

  /* eQUIC syncronization interval */
  int sync_interval;
};

/* Create a public type to create equic_meta struct */
typedef struct equic_meta equic_t;



/**
 *
 * Public functions of eQUIC module
 *
 */
static void equic_sigint_callback (int signal);

static void equic_sigterm_callback (int signal);

static void equic_get_interface (equic_t *equic, char if_name[]);

static void equic_read_object (equic_t *equic, char bpf_prog_path[]);

static void equic_load (equic_t *equic);

static void equic_sync_counters (int map_fd, int interval);


#endif /* EQUIC_H */
