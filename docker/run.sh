#!/bin/bash

CURRENT_UID=$(id -u)
CURRENT_DIR=$(pwd)
CONTAINER_NAME="ubuntu20.04-nes-$CURRENT_UID"

docker run -it --rm \
    -u $CURRENT_UID \
    -v "$CURRENT_DIR":/home/user/work \
    "$CONTAINER_NAME"
