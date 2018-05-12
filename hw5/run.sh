#!/bin/bash

g++ -o multi-lookup *.cpp -std=c++11 -Wall -Wno-vla -pedantic -Os -lpthread -lmyutil -L $LD_LIBRARY_PATH
