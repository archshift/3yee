#!/bin/bash

cpu=4

mkdir -p embuild
cd embuild
emconfigure cmake .. -DEMSCRIPTEN=1
emmake make -j$cpu