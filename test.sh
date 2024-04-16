#!/bin/bash

FIFO_NAME="fifo_cl_sv"
CMD_TYPES=("execute")
ARGS_LIST=("ls -l" "date" "ps aux" "df -h")
NUM_CLIENTS=3
INTERVAL=1.5  # Average interval in seconds

while true; do
    for (( i=1; i<=NUM_CLIENTS; i++ )); do
        # Build instruction
        cmd_type=${CMD_TYPES[$RANDOM % ${#CMD_TYPES[@]}]}
        args=${ARGS_LIST[$RANDOM % ${#ARGS_LIST[@]}]}
        instruction="CommandType: $cmd_type\nTime: 5\nIsPipeline: -u\nArgs: $args\n"

        # Send instruction (background to avoid blocking)
        echo "$instruction" > "$FIFO_NAME" &  

        # Introduce randomized delay
        sleep $(echo "scale=2; $INTERVAL + $RANDOM / 32767" | bc) 
    done
done