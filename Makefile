
#
# Variables block
#

DOCKERFILES_DIR := docker

DOCKER_TAG ?= local

DOCKER_CMD := $(shell which docker)

DOCKER_COMPOSE_CMD := $(shell which docker-compose)


# Compiler optimization options
OPTMIZATIONS := -O2

# C compilation flags
CFLAGS := -DDEBUG -D__BPF_TRACING__ -D__KERNEL__
CFLAGS += $(OPTMIZATIONS)

# Headers include path for Kernel module
INCLUDE_PATH := /usr/include/x86_64-linux-gnu/

# C lang compiler
CC := $(shell which clang)

# Source code directory
SRC := src

# Binary directory
BIN := bin

# Logs directory
LOGS := logs

# Log suffixes used to name files
LOG_SUFFIX ?= baseline
KERNEL_LOG_SUFFIX ?= kernel

# Kernel trace file which eBPF prints debug messages
TRACE_PIPE := /sys/kernel/debug/tracing/trace_pipe

# Network interface to attach eBPF program
IFACE ?= eth0

# Current timestamp
NOW := $(shell date +%s)

# Load test request size
REQ_SIZE ?= 64k

#
# Checks if trace file exist. If not, mount it
#
define check_debugfs

	@stat $(TRACE_PIPE) > /dev/null \
		&& echo "Trace found" \
		|| (echo "Mounting debugfs.." \
			&& mount -t debugfs none /sys/kernel/debug)
endef


#
# Target rules block
#
.DEFAULT_GOAL: help


help: greetings
	@echo "eQUIC available target rules"
	@echo
	@echo "-- Host machine --"
	@echo " build               Builds docker image"
	@echo " start               Starts docker compose"
	@echo " stop                Stops docker compose"
	@echo " restart             Restarts docker containers"
	@echo " client1_shell       Runs a shell on the client 1 container"
	@echo " server_shell        Runs a shell on the server container"
	@echo " logs                Tails the server application logs"
	@echo
	@echo "-- Inside the Container --"
	@echo " compile             Compiles the eQUIC for kernel and userspace"
	@echo " kernel              Compiles the eBPF program for kernel"
	@echo " userspace           Compiles the eQUIC user space program"
	@echo " library             Builds eQUIC as a shared library"
	@echo " echo                Builds lsquic echo server with eQUIC"
	@echo " http                Builds lsquic http server with eQUIC"
	@echo " clean               Remove all files resulted by the compilations"
	@echo " clean_logs          Remove all log files from logging directory"
	@echo " load                Loads the eBPF program into interface"
	@echo " unload              Unloads the eBPF program into interface"
	@echo " debug               Tails the eBPF program logs (trace_pipe)"
	@echo " show                Shows the network interface link state"
	@echo " bpf_dev             Runs all targets to build load and debug eBPF"
	@echo " bpf                 Runs all targets to build load eBPF program"
	@echo " run_server          Runs the lsquic echo binary as a server"
	@echo " run_client          Runs the lsquic echo binary as a client"
	@echo " http_server         Runs the lsquic http binary as a server"
	@echo " http_client         Runs the lsquic http binary as a client"
	@echo


greetings:
	@echo "        ___  _   _ ___ ____ "
	@echo "   ___ / _ \| | | |_ _/ ___|"
	@echo "  / _ \ | | | | | || | |    "
	@echo " |  __/ |_| | |_| || | |___ "
	@echo "  \___|\__\_\\\\\___/|___\____|"
	@echo


# Builds docker images locally
build: greetings $(DOCKERFILES_DIR)/Dockerfile
	$(info Building eQUIC docker image..)
	@$(DOCKER_CMD) build --tag equic:$(DOCKER_TAG) --file $(word 2, $^) --no-cache --memory=8g --cpuset-cpus=0,1,2,3 .


start: $(DOCKERFILES_DIR)/docker-compose.yaml
	$(info Running eQUIC client and server..)
	@$(DOCKER_COMPOSE_CMD) --file $< up --detach


stop: $(DOCKERFILES_DIR)/docker-compose.yaml
	$(info Stopping eQUIC client and server..)
	@$(DOCKER_COMPOSE_CMD) --file $< down --remove-orphans


restart: stop start


client1_shell:
	@docker exec -it equic-client-1 bash

server_shell:
	@docker exec -it equic-server bash

logs:
	@$(DOCKER_CMD) logs --tail 100 --follow equic-server


#
# Targets to run XDP related tasks
#
compile: greetings kernel userspace

kernel: $(SRC)/equic_kern.c
	$(info Compiling eBPF kernel program)
	$(CC) -target bpf -c $< -o $(BIN)/equic_kern.o -I $(INCLUDE_PATH) $(CFLAGS)

userspace: library
	$(info Compiling eQUIC userspace program standalone program)
	$(CC) $(SRC)/equic.c $(BIN)/equic_user.o -O2 -lbpf -lelf -lz -o $(BIN)/equic

library: $(SRC)/equic_user.c
	$(info Compiling eQUIC userspace as static library)
	gcc -c $< -fPIE -O2 -o $(BIN)/equic_user.o

echo:
	$(info Compiling lsquic echo server with eQUIC offload)
	@cd /src/lsquic && make echo_server

http:
	$(info Compiling lsquic HTTP server with eQUIC offload)
	@cd /src/lsquic && make http_server

clean:
	@rm -rvf $(BIN)/equic $(wildcard $(BIN)/*.o)

clean_logs:
	@rm -rvf $(LOGS)/*.log

load:
	$(info Loading eBPF program on interface $(IFACE))
	ip link set dev $(IFACE) xdp obj $(BIN)/equic_kern.o sec equic

unload:
	$(info Unloading eBPF program from interface $(IFACE))
	ip link set dev $(IFACE) xdp off

debug:
	$(info Entering debug mode)
	$(call check_debugfs)
	@cat $(TRACE_PIPE) | tee $(LOGS)/$(shell date +%s)-$(KERNEL_LOG_SUFFIX).log

show:
	@ip link show dev $(IFACE)

bpf_dev: greetings unload compile load debug

bpf: greetings unload compile load

run_server: /src/lsquic/echo_server
	$(eval SSL_DIR=/src/equic/ssl)
	$< -c localhost,$(SSL_DIR)/cert.pem,$(SSL_DIR)/private.key

run_client: /src/lsquic/echo_client
	$(eval SRV_ADDR=$(shell host equic-server | awk '{print $$4}'))
	$< -H localhost -s $(SRV_ADDR):12345

http_server: /src/lsquic/http_server
	$(eval SSL_DIR=/src/equic/ssl)
	$< -c localhost,$(SSL_DIR)/cert.pem,$(SSL_DIR)/private.key > $(LOGS)/$(NOW)-$(LOG_SUFFIX).log

http_client: /src/lsquic/http_client
	$(eval SRV_ADDR=$(shell host equic-server | awk '{print $$4}'))
	$< -H localhost -s $(SRV_ADDR):12345 -p /$(REQ_SIZE)

load_test: $(DOCKERFILES_DIR)/docker-compose-load-clients.yaml
	$(info Running eQUIC load test..)
	REQ_SIZE=$(REQ_SIZE) $(DOCKER_COMPOSE_CMD) --file $< up
	$(DOCKER_COMPOSE_CMD) --file $< down
