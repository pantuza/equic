#ifndef EQUIC_H
#define EQUIC_H

#include <arpa/inet.h>


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
void equic_sigint_callback (int signal);

void equic_sigterm_callback (int signal);

void equic_get_interface (equic_t *equic, char if_name[]);

void equic_read_object (equic_t *equic, char bpf_prog_path[]);

void equic_load (equic_t *equic);

void equic_sync_counters (equic_t *equic, const struct sockaddr *client);


#endif /* EQUIC_H */
