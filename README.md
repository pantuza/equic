# eQUIC

QUIC protocol connection quota control offload to eBPF + XDP


### Dependencies

We use docker to run a QUIC server and two container clients to trigger
requests.

### Getting started

This project uses `Makefile` to automate small tasks. Try running
`make help` at the project root directory to see all available target rules.


First of all, you should build the docker images to run the applications.
To do it, run as follows:

```bash
$> make build
```

To run the containers we hava a `docker/docker-compose.yaml` file to that.
In order to run one server and two client containers do as follows:

```bash
$> make start
```

Once the containers are ready and runnning you are able to get a shell inside
each of the containers. When you get a shell, you'll be placed at the eQUIC
project directory. Thus, you can use the Makefile again. Run `make help` and
check all target rules available to be runned from inside a container.

```bash
# Get a shell on the server
$> make server_shell
# Get a shell on the client
$> make client_shell
```

From inside the server container, compile the eQUIC library and kernel module.

```bash
$server> make clean compile
```

Once you have the kernel module and library ready, you can compile the server
program:

```bash
$server> make echo
```

Then run the server binary that will load the kernel module at runtime:
```bash
$server> make run_server
/src/lsquic/echo_server -c localhost,/src/equic/ssl/cert.pem,/src/equic/ssl/private.key
[eQUIC] Action=Read, Type=OS, Interface=182
[eQUIC] Action=Load, Type=BPF, Object=/src/equic/bin/equic_kern.o
[eQUIC] Action=Load, Type=BPF, Map=counters
[eQUIC] Action=Setup, Type=BPF, Hook=XDP
```

All right! To try connecting to the server with a client, open a shell on one
of the client containers using `make client1_shell` and then run as follows
from inside the container:

```bash
$client1> make run_client
```

Then you can type anything and once you hit Return, the packet will be sent
to the server and ECHOED back to you using QUIC protocol. Also, if you reach
the Quota limit defined inside the eQUIC library your packets are going to be
droped directly on the kernel.


### Running HTTP client and server

On the server container run:

```bash
$> make http  # compiles http code
$> make http_server  # Runs the web server
```

On client container run:

```bash
$> make http_client # Runs the client application that triggers requests to server
```


### Running experiments

Make sure you have the http server running inside the server container:

```bash
$> make http_server
```

On the host machine run:

```bash
$> make load_test REQ_SIZE=4k
```

It will spam clients sending requests to the server.
If you want to run experiments in parallel, do as follows:

```bash
make parallel_load_test REQ_SIZE=1k
```
It will also spam clients sending requests to the server, but now in parallel

> Other experiments can be executed following [experiments/README.md](experiments/README.md)
