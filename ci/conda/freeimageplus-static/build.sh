#!/bin/bash

cp $RECIPE_DIR/Makefile.osx .

# Build step 
make -j $CPU_COUNT
make install

