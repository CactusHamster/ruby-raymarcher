#!/bin/env bash
target=$1
if [ -z "${target}" ]; then
    target="default"
fi


if [[ $target == "build" ]]; then
    make -C ext/opengl
    make -C ext/glfw
elif [[ $target == "debug" ]]; then
    make -C ext/opengl CFLAGS="-g -Wall" && \
    make -C ext/glfw CFLAGS="-g -Wall" && \
    gdb --args ruby main.rb
elif [[ $target == "default" ]]; then
    make -C ext/opengl && \
    make -C ext/glfw && \
    ruby main.rb 
else
    echo "Unknown target \"${target}\"."
fi