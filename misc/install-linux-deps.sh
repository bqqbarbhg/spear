#!/usr/bin/env bash

if hash apt 2>/dev/null
then
    sudo apt install xorg-dev libcurl4-openssl-dev libgl1-mesa-dev clang
elif hash pacman 2>/dev/null
then
    sudo pacman -S base-devel curl mesa libxcursor clang
else
    echo "No supported package manager found."
fi
