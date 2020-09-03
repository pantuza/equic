/* Copyright (c) 2017 - 2020 LiteSpeed Technologies Inc.  See LICENSE. */
/*
 * echo_server.c -- QUIC server that echoes back input line by line
 */


/*
 * This code is a modified version of the original echo_server.c
 * from LiteSpeed QUIC library for research purpose.
 *
 * This implementation adds eQUIC library to load a eBPF program on
 * the XDP kernel hook. On every new QUIC connection setup and QUIC
 * connection teardown this program updates a eBPF kernel Map.
 *
 * Take a look at the code in the following functions:
 *   . debug_connection
 *   . echo_server_on_new_conn
 *   . echo_server_on_conn_closed
 *   . main
 *
 * Author: Gustavo Pantuza <gustavopantuza@gmail.com>
 */

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <time.h>
#include <arpa/inet.h>
#ifndef WIN32
#include <unistd.h>
#include <netinet/in.h>
#else
#include "vc_compat.h"
#include "getopt.h"
#endif

#include "lsquic.h"
#include "test_common.h"
#include "prog.h"

#include "../src/liblsquic/lsquic_logger.h"

#include "/src/equic/src/equic_user.h"

struct lsquic_conn_ctx;

struct echo_server_ctx {
    TAILQ_HEAD(, lsquic_conn_ctx)   conn_ctxs;
    unsigned max_reqs;
    int n_conn;
    struct sport_head sports;
    struct prog *prog;

    equic_t *equic;
};

struct lsquic_conn_ctx {
    TAILQ_ENTRY(lsquic_conn_ctx)    next_connh;
    lsquic_conn_t       *conn;
    struct echo_server_ctx   *server_ctx;
};

/**
 * Prints a 4-tuple with Source IPv4 address and port and Destination
 * IPv4 address and port
 */
static void
debug_connection (const struct sockaddr *local, const struct sockaddr *peer, char *action)
{
    /* Gets server ip address and port */
    struct sockaddr_in *local_addr_in = (struct sockaddr_in *)local;
    char *local_addr_str = inet_ntoa(local_addr_in->sin_addr);
    uint16_t local_port;
    local_port = htons(local_addr_in->sin_port);

    /* Gets client ip address and port */
    struct sockaddr_in *peer_addr_in = (struct sockaddr_in *)peer;
    char *peer_addr_str = inet_ntoa(peer_addr_in->sin_addr);
    uint16_t peer_port;
    peer_port = htons(peer_addr_in->sin_port);

    printf(
        "[eQUIC] Action=%s, Client=%s, SourcePort=%d, Server=%s, DestinationPort=%d\n",
        action, peer_addr_str, peer_port, local_addr_str, local_port
    );
}


static lsquic_conn_ctx_t *
echo_server_on_new_conn (void *stream_if_ctx, lsquic_conn_t *conn)
{
    struct echo_server_ctx *server_ctx = stream_if_ctx;
    lsquic_conn_ctx_t *conn_h = calloc(1, sizeof(*conn_h));
    conn_h->conn = conn;
    conn_h->server_ctx = server_ctx;
    TAILQ_INSERT_TAIL(&server_ctx->conn_ctxs, conn_h, next_connh);
    LSQ_NOTICE("New connection!");
    print_conn_info(conn);

    /* Read sockaddr from connection for client and server */
    const struct sockaddr *local;
    const struct sockaddr *peer;
    lsquic_conn_get_sockaddr(conn, &local, &peer);

    debug_connection(local, peer, "ConnectionSetup");

    /*
     * Increment eQUIC Kernel counters Map key for the give peer (client)
     */
    equic_inc_counter(server_ctx->equic, peer);

    return conn_h;
}


static void
echo_server_on_conn_closed (lsquic_conn_t *conn)
{
    /* Read sockaddr from connection for client and server */
    const struct sockaddr *local;
    const struct sockaddr *peer;
    lsquic_conn_get_sockaddr(conn, &local, &peer);

    lsquic_conn_ctx_t *conn_h = lsquic_conn_get_ctx(conn);
    if (conn_h->server_ctx->n_conn)
    {
        --conn_h->server_ctx->n_conn;
        LSQ_NOTICE("Connection closed, remaining: %d", conn_h->server_ctx->n_conn);
        if (0 == conn_h->server_ctx->n_conn)
            prog_stop(conn_h->server_ctx->prog);
    }
    else
        LSQ_NOTICE("Connection closed");
    TAILQ_REMOVE(&conn_h->server_ctx->conn_ctxs, conn_h, next_connh);
    free(conn_h);

    debug_connection(local, peer, "ConnectionClose");

    /*
     * Decrement eQUIC Kernel counters Map key for the give peer (client)
     */
    equic_dec_counter(conn_h->server_ctx->equic, peer);
}


struct lsquic_stream_ctx {
    lsquic_stream_t     *stream;
    struct echo_server_ctx   *server_ctx;
    char                 buf[0x100];
    size_t               buf_off;
};


static lsquic_stream_ctx_t *
echo_server_on_new_stream (void *stream_if_ctx, lsquic_stream_t *stream)
{
    lsquic_stream_ctx_t *st_h = malloc(sizeof(*st_h));
    st_h->stream = stream;
    st_h->server_ctx = stream_if_ctx;
    st_h->buf_off = 0;
    lsquic_stream_wantread(stream, 1);
    return st_h;
}


static struct lsquic_conn_ctx *
find_conn_h (const struct echo_server_ctx *server_ctx, lsquic_stream_t *stream)
{
    struct lsquic_conn_ctx *conn_h;
    lsquic_conn_t *conn;

    conn = lsquic_stream_conn(stream);
    TAILQ_FOREACH(conn_h, &server_ctx->conn_ctxs, next_connh)
        if (conn_h->conn == conn)
            return conn_h;
    return NULL;
}


static void
echo_server_on_read (lsquic_stream_t *stream, lsquic_stream_ctx_t *st_h)
{
    struct lsquic_conn_ctx *conn_h;
    size_t nr;

    nr = lsquic_stream_read(stream, st_h->buf + st_h->buf_off++, 1);
    if (0 == nr)
    {
        LSQ_NOTICE("EOF: closing connection");
        lsquic_stream_shutdown(stream, 2);
        conn_h = find_conn_h(st_h->server_ctx, stream);
        lsquic_conn_close(conn_h->conn);
    }
    else if ('\n' == st_h->buf[ st_h->buf_off - 1 ])
    {
        /* Found end of line: echo it back */
        lsquic_stream_wantwrite(stream, 1);
        lsquic_stream_wantread(stream, 0);
    }
    else if (st_h->buf_off == sizeof(st_h->buf))
    {
        /* Out of buffer space: line too long */
        LSQ_NOTICE("run out of buffer space");
        lsquic_stream_shutdown(stream, 2);
    }
    else
    {
        /* Keep reading */;
    }
}


static void
echo_server_on_write (lsquic_stream_t *stream, lsquic_stream_ctx_t *st_h)
{
    lsquic_stream_write(stream, st_h->buf, st_h->buf_off);
    st_h->buf_off = 0;
    lsquic_stream_flush(stream);
    lsquic_stream_wantwrite(stream, 0);
    lsquic_stream_wantread(stream, 1);
}


static void
echo_server_on_stream_close (lsquic_stream_t *stream, lsquic_stream_ctx_t *st_h)
{
    struct lsquic_conn_ctx *conn_h;
    LSQ_NOTICE("%s called", __func__);
    conn_h = find_conn_h(st_h->server_ctx, stream);
    LSQ_WARN("%s: TODO: free connection handler %p", __func__, conn_h);
    free(st_h);
}


const struct lsquic_stream_if server_echo_stream_if = {
    .on_new_conn            = echo_server_on_new_conn,
    .on_conn_closed         = echo_server_on_conn_closed,
    .on_new_stream          = echo_server_on_new_stream,
    .on_read                = echo_server_on_read,
    .on_write               = echo_server_on_write,
    .on_close               = echo_server_on_stream_close,
};


static void
usage (const char *prog)
{
    const char *const slash = strrchr(prog, '/');
    if (slash)
        prog = slash + 1;
    printf(
"Usage: %s [opts]\n"
"\n"
"Options:\n"
                , prog);
}


int
main (int argc, char **argv)
{
    int opt, s;
    struct prog prog;
    struct echo_server_ctx server_ctx;

    memset(&server_ctx, 0, sizeof(server_ctx));
    server_ctx.prog = &prog;
    TAILQ_INIT(&server_ctx.sports);
    TAILQ_INIT(&server_ctx.conn_ctxs);

    /* eQUIC initialization and setup */
    equic_t equic;
    equic_get_interface(&equic, "eth0");
    equic_read_object(&equic, "/src/equic/bin/equic_kern.o");

    signal(SIGINT, equic_sigint_callback); 
    signal(SIGTERM, equic_sigterm_callback);

    equic_load(&equic);

    server_ctx.equic = &equic;

    prog_init(&prog, LSENG_SERVER, &server_ctx.sports,
                                        &server_echo_stream_if, &server_ctx);

    while (-1 != (opt = getopt(argc, argv, PROG_OPTS "hn:")))
    {
        switch (opt) {
        case 'n':
            server_ctx.n_conn = atoi(optarg);
            break;
        case 'h':
            usage(argv[0]);
            prog_print_common_options(&prog, stdout);
            exit(0);
        default:
            if (0 != prog_set_opt(&prog, opt, optarg))
                exit(1);
        }
    }

    if (0 != prog_prep(&prog))
    {
        LSQ_ERROR("could not prep");
        exit(EXIT_FAILURE);
    }

    LSQ_DEBUG("entering event loop");

    s = prog_run(&prog);
    prog_cleanup(&prog);

    exit(0 == s ? EXIT_SUCCESS : EXIT_FAILURE);
}
