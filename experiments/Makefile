#
# This makefile is responsible to rule all data analysis tasks
#


#
# VARIABLES
#
LOGS_DIR := ../logs

IN_DIR := in
OUT_DIR := out
STATS_DIR := stats

# Python 3 binary path
PYTHON3 := $(shell which python3)

# Gnu Plot binary path
GNUPLOT := $(shell which gnuplot)

#
# TARGET RULES
#

.DEFAULT_GOAL: help

.PHONY: help
help:
	@echo "===== eQUIC analysis ====="
	@echo
	@echo "Available target rules:"
	@echo
	@echo "extract          Extracts data from log files"
	@echo "compute          Compute and summarize data into statistics"
	@echo "aggregate        Aggregate statistics into single files"
	@echo "plot             Plots results using gnu plot"
	@echo "show             Shows the graph results"
	@echo "clean            Cleans experiments resulted files"
	@echo
	@echo "--"
	@echo


.PHONY: extract
extract:
	$(info === Extracting data from logs ===)
	@$(PYTHON3) extract.py $(LOGS_DIR)/*-baseline-256k.log
	@$(PYTHON3) extract.py $(LOGS_DIR)/*-userspace-256k.log
	@$(PYTHON3) extract.py $(LOGS_DIR)/*-kernel-256k.log
	@$(PYTHON3) extract.py $(LOGS_DIR)/*-parallel_cpu_userspace-256k.log
	@$(PYTHON3) extract.py $(LOGS_DIR)/*-parallel_cpu_kernel-256k.log
	@echo
	

.PHONY: compute
compute:
	$(info === Summarizing statistics from data ===)
	@$(PYTHON3) compute.py $(IN_DIR)/duration-baseline-256k.dat
	@$(PYTHON3) compute.py $(IN_DIR)/duration-userspace-256k.dat
	@$(PYTHON3) compute.py $(IN_DIR)/duration-kernel-256k.dat
	@$(PYTHON3) compute.py $(IN_DIR)/reqs_per_second-baseline-256k.dat
	@$(PYTHON3) compute.py $(IN_DIR)/reqs_per_second-userspace-256k.dat
	@$(PYTHON3) compute.py $(IN_DIR)/reqs_per_second-kernel-256k.dat
	@$(PYTHON3) compute.py $(IN_DIR)/block_duration-parallel_cpu_kernel-256k.dat
	@$(PYTHON3) compute.py $(IN_DIR)/block_duration-parallel_cpu_userspace-256k.dat
	@$(PYTHON3) cpu.py
	@$(PYTHON3) mem.py
	@echo


.PHONY: aggregate
aggregate:
	$(info === Aggregating statistics ===)
	@$(PYTHON3) aggregate.py
	@echo


.PHONY: plot
plot:
	$(info === Plotting graphs from statistics ===)
	@$(GNUPLOT) graphs.plt
	@echo


.PHONY: show
show:
	$(info === Showing results graphs ===)
	qlmanage -p $(OUT_DIR)/duration.png
	qlmanage -p $(OUT_DIR)/requests.png
	qlmanage -p $(OUT_DIR)/block-duration.png
	qlmanage -p $(OUT_DIR)/requests-per-second.png
	qlmanage -p $(OUT_DIR)/cpu-cdf.png
	qlmanage -p $(OUT_DIR)/mem-cdf.png


clean:
	@rm -rvf $(IN_DIR)/*.dat
	@rm -rvf $(STATS_DIR)/*.csv
	@rm -rvf $(OUT_DIR)/*.png
