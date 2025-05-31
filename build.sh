#!/bin/bash
git clone https://github.com/magiblot/tvision.git
mkdir build
cd build
cmake ..
make -j$(nproc --all)
