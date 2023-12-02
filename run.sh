#!/bin/env bash
target=$1
if [ -z "${target}" ]; then
    target="default"
fi


if [[ $target == "build" ]]; then
    make -C ext/window_utils
elif [[ $target == "debug" ]]; then
    make -C ext/window_utils CFLAGS="--verbose -Wall" && \
    ruby main.rb
elif [[ $target == "default" ]]; then
    make -C ext/window_utils && \
    ruby main.rb 
else
    echo "Unknown target \"${target}\"."
fi