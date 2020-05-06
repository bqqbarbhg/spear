#!/usr/bin/env bash

if which premake5
then
	echo "Found global premake5"
elif [ -f premake5 ]
then
	echo "Found local ./premake5"
else
	echo "Downloading premake5 to local ./premake5"
	curl -L https://github.com/premake/premake-core/releases/download/v5.0.0-alpha15/premake-5.0.0-alpha15-linux.tar.gz -o premake5.tar.gz
	tar -xf premake5.tar.gz
	rm premake5.tar.gz
fi

sudo apt install xorg-dev libcurl4-openssl-dev libgl1-mesa-dev

