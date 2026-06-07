#!/bin/bash

# Permite o container usar o display do host
xhost +local:docker

docker build -t projetops-ui .

docker run --rm \
    -e DISPLAY=$DISPLAY \
    -e QT_STYLE_OVERRIDE=Fusion \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    projetops-ui
