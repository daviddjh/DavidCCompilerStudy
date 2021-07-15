#!/bin/bash

# Change working directory
cd /home/daviddjh/dev/DavidCCompilerStudy/DavidCCompilerStudy

# Compile the files into ../Build/Debug/
make

# Change working directory to Build/Debug folder
cd /home/daviddjh/dev/DavidCCompilerStudy/Build/Debug

# Run program that was just built
./SampleCompiler
