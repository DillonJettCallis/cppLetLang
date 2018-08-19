#!/bin/sh

llc -filetype=obj -o build/basic.o build/basic.ll

clang++ -o build/out build/core.o build/basic.o