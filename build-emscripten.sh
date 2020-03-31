#!/bin/bash

cpu=4

mkdir -p embuild
cd embuild
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release -DEMSCRIPTEN=1
emmake make -j$cpu
