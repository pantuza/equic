
DOCKERFILES_DIR := docker

DOCKER_TAG ?= local

DOCKER_CMD := $(shell which docker)

DOCKER_COMPOSE_CMD := $(shell which docker-compose)


.DEFAULT_GOAL: help


help:
	@echo "eQUIC available target rules"


# Builds all docker images locally
build: build_server build_client


build_server: $(DOCKERFILES_DIR)/Dockerfile.server
	$(info Building eQUIC server docker image..)
	@$(DOCKER_CMD) build --tag equic-server:$(DOCKER_TAG) --file $< --no-cache .


build_client: $(DOCKERFILES_DIR)/Dockerfile.client
	$(info Building eQUIC client docker image..)
	@$(DOCKER_CMD) build --tag equic-client:$(DOCKER_TAG) --file $< --no-cache .

build_mvfst: $(DOCKERFILES_DIR)/Dockerfile.client
	$(info Building eQUIC docker image with mvfst..)
	@$(DOCKER_CMD) build --tag equic:$(DOCKER_TAG) --file $< --no-cache --memory=8g --cpuset-cpus=0,1,2,3 .

start: $(DOCKERFILES_DIR)/docker-compose.yaml
	$(info Running eQUIC client and server..)
	@$(DOCKER_COMPOSE_CMD) --file $< up --detach


stop: $(DOCKERFILES_DIR)/docker-compose.yaml
	$(info Stopping eQUIC client and server..)
	@$(DOCKER_COMPOSE_CMD) --file $< down --remove-orphans


restart: stop start


restart_server:
	$(info Restarting eQUIC server container..)
	@$(DOCKER_CMD) restart equic-server


restart_client:
	$(info Restarting eQUIC client container..)
	@$(DOCKER_CMD) restart equic-client


client_shell:
	@docker exec -it equic-client bash

server_shell:
	@docker exec -it equic-server bash

logs:
	@$(DOCKER_CMD) logs --tail 100 --follow equic-server


#
# Targets to run XDP related tasks
#
compile:
	clang -target bpf -O2 -c drop.c -o drop.o -I /libbpf/src/ -DDEBUG -D__BPF_TRACING__ -D__KERNEL__

load:
	ip link set dev eth0 xdp obj drop.o sec drop

unload:
	ip link set dev eth0 xdp off

debug:
	@cat /sys/kernel/debug/tracing/trace_pipe

bpf_dev: unload compile load debug

bpf: unload compile load
