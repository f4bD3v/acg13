#!/bin/sh

# this absolute directory
SCRIPT_PATH=$( cd $(dirname $0); pwd -P)

# compile if needed
# TODO check for the need to compile
qmake-qt4 ../nori.pro && make
