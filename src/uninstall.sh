#!/bin/bash

cd tracker
make clean
cd ../id
make clean
cd ../storage
make clean
cd ../client
make clean 
cd ../http
make clean
cd ../../bin
rm -rf *

exit 0