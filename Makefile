
DOCKERFILES_DIR := docker

DOCKER_TAG ?= local

DOCKER_CMD := $(shell which docker)


# Builds all docker images locally
build: build_server build_client


build_server: $(DOCKERFILES_DIR)/Dockerfile.server
	$(info Building eQUIC server docker image..)
	@$(DOCKER_CMD) build --tag equic-server:$(DOCKER_TAG) --file $< --no-cache .


build_client: $(DOCKERFILES_DIR)/Dockerfile.client
	$(info Building eQUIC client docker image..)
	@$(DOCKER_CMD) build --tag equic-client:$(DOCKER_TAG) --file $< --no-cache .
