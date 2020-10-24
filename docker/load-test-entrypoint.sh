#!/bin/bash

# Number of request to be done to the remote server
N_REQS=1000;


# Default size of the payload expected in the response
if [ ! -z "$1" ]; then
    REQ_SIZE="64k";
fi;


# Current container name
HOSTNAME=$(hostname)

#
# Load test execution loop
#
for i in $(seq ${N_REQS}); do

    echo "Request $i from $HOSTNAME with response size $REQ_SIZE"
    make http_client REQ_SIZE=${REQ_SIZE} > /dev/null;
done;
