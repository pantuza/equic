
APP_NAME := eQUIC BPFTrace scripts


.PHONY: help
help:  ## Displays help message
	@echo "Makefile to control development tasks for $(APP_NAME)"
	@awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n  make \033[36m<target>\033[0m\n\nTargets:\n"} /^[a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-20s\033[0m %s\n", $$1, $$2 }' $(MAKEFILE_LIST)


.PHONY: page_fault
page_fault: ## Count page faults by process
	bpftrace -e 'software:faults:1 { @[comm] = count(); }'


.PHONY: syscall_rate
syscall_rate: ## Show per-second syscall rates
	bpftrace -e 'tracepoint:raw_syscalls:sys_enter { @ = count(); } interval:s:1 { print(@); clear(@); }'


.PHONY: read_distribution
read_distribution: ## Read size distribution by proces
	bpftrace -e 'tracepoint:syscalls:sys_exit_read { @[comm] = hist(args->ret); }'


.PHONY: cpu_sched_latency
cpu_sched_latency: ## CPU scheduler run queue latency as a histogram
	./runqlat.bt


.PHONY: packet_drops
packet_drops: ## Trace kernel-based TCP packet drops with details
	./tcpdrop.bt


.PHONY: block_io_latency
block_io_latency: ## Block I/O latency as a histogram
	./biolatency.bt
