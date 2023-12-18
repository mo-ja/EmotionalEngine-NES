#!/bin/bash

CURRENT_UID=$(id -u)
CONTAINER_NAME="ubuntu20.04-nes-$CURRENT_UID"

docker build -t "$CONTAINER_NAME" .
