
APP_NAME := eQUIC BPFTrace scripts

TRACES_DIR := ../logs/traces


.PHONY: help
help:  ## Displays help message
	@echo "Makefile to control development tasks for $(APP_NAME)"
	@awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n  make \033[36m<target>\033[0m\n\nTargets:\n"} /^[a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-20s\033[0m %s\n", $$1, $$2 }' $(MAKEFILE_LIST)


.PHONY: page_fault
page_fault: ## Count page faults by process
	bpftrace -e 'software:faults:1 { @[comm] = count(); }' > $(TRACES_DIR)/$@-$(shell date +%s).trace


.PHONY: syscall_rate
syscall_rate: ## Show per-second syscall rates
	bpftrace -e 'tracepoint:raw_syscalls:sys_enter { @ = count(); } interval:s:1 { print(@); clear(@); }' > $(TRACES_DIR)/$@-$(shell date +%s).trace


.PHONY: read_distribution
read_distribution: ## Read size distribution by proces
	bpftrace -e 'tracepoint:syscalls:sys_exit_read { @[comm] = hist(args->ret); }' > $(TRACES_DIR)/$@-$(shell date +%s).trace


.PHONY: cpu_sched_latency
cpu_sched_latency: ## CPU scheduler run queue latency as a histogram
	./runqlat.bt > $(TRACES_DIR)/$@-$(shell date +%s).trace


.PHONY: packet_drops
packet_drops: ## Trace kernel-based TCP packet drops with details
	./tcpdrop.bt > $(TRACES_DIR)/$@-$(shell date +%s).trace


.PHONY: block_io_latency
block_io_latency: ## Block I/O latency as a histogram
	./biolatency.bt > $(TRACES_DIR)/$@-$(shell date +%s).trace


.PHONY: clean_traces
clean_traces: ## Cleans all traces logs
	rm -rf $(TRACES_DIR)/*.trace
