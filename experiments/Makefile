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

help:
	@echo "===== eQUIC analysis ====="
	@echo
	@echo "Available target rules:"
	@echo
	@echo "extract          Extracts data from log files"
	@echo "compute          Compute and summarize data into statistics"
	@echo "plot             Plots results using gnu plot"
	@echo "show             Shows the graph results"
	@echo "clean            Cleans experiments resulted files"
	@echo
	@echo "--"
	@echo


extract:
	$(info Extracting data from logs)
	@$(PYTHON3) extract.py $(LOGS_DIR)/*-baseline-16k.log
	@$(PYTHON3) extract.py $(LOGS_DIR)/*-baseline-32k.log
	@$(PYTHON3) extract.py $(LOGS_DIR)/*-baseline-64k.log
	@$(PYTHON3) extract.py $(LOGS_DIR)/*-baseline-128k.log
	@$(PYTHON3) extract.py $(LOGS_DIR)/*-baseline-256k.log
	@$(PYTHON3) extract.py $(LOGS_DIR)/*-baseline-512k.log
	@$(PYTHON3) extract.py $(LOGS_DIR)/*-baseline-1024k.log


compute:
	$(info Summarizing statistics from data)
	@$(PYTHON3) compute.py $(IN_DIR)/*-baseline-16k.dat
	@$(PYTHON3) compute.py $(IN_DIR)/*-baseline-32k.dat
	@$(PYTHON3) compute.py $(IN_DIR)/*-baseline-64k.dat
	@$(PYTHON3) compute.py $(IN_DIR)/*-baseline-128k.dat
	@$(PYTHON3) compute.py $(IN_DIR)/*-baseline-256k.dat
	@$(PYTHON3) compute.py $(IN_DIR)/*-baseline-512k.dat
	@$(PYTHON3) compute.py $(IN_DIR)/*-baseline-1024k.dat


plot:
	$(info Plotting graphs from statistics)
	@$(GNUPLOT) graphs.plt


show:
	$(info Showing results graphs)
	qlmanage -p $(OUT_DIR)/duration.png


clean:
	@rm -rvf $(IN_DIR)/*.dat
	@rm -rvf $(STATS_DIR)/*.csv
	@rm -rvf $(OUT_DIR)/*.png
