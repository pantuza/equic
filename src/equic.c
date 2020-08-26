#include <stdlib.h>
#include <signal.h>

#include "equic_user.h"


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
