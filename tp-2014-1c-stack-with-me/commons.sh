#!/bin/bash

cd $HOME

git clone https://github.com/sisoputnfrba/so-commons-library.git

cd $HOME/so-commons-library

make all

sudo make install
