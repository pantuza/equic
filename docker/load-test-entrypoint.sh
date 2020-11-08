#!/bin/bash

# Number of request to be done to the remote server
N_REQS=1000;


# Default size of the payload expected in the response
if [ -z "${REQ_SIZE}" ]; then
    REQ_SIZE="64k";
fi;
echo "Load test will ask for responses with with ${REQ_SIZE}"


# Current container name
HOSTNAME=$(hostname)

# Time to wait before triggering parallel load
SLEEP_TIME=30

#
# Checks for parallel execution. Clients that triggers parallel
# requests are the ones that reach the quota limit. In other words,
# they are the attacker containers
#
if [[ ${PARALLEL} -eq 1 ]]; then

    echo "Going to run in parallel in ${SLEEP_TIME} seconds.."

    # Attacker containers waits 30 seconds before starting
    # sending load to the server
    sleep ${SLEEP_TIME};
fi;

#
# Load test execution loop
#
for i in $(seq ${N_REQS}); do


    if [[ ${PARALLEL} -eq 1 ]]; then
        echo "Parallel request $i from $HOSTNAME with response size $REQ_SIZE"
        make http_client REQ_SIZE=${REQ_SIZE} > /dev/null &

    else
        echo "Request $i from $HOSTNAME with response size $REQ_SIZE"
        make http_client REQ_SIZE=${REQ_SIZE} > /dev/null
    fi;
done;
