#!/bin/bash

cd $HOME

git clone https://github.com/sisoputnfrba/ansisop-parser.git

cd $HOME/ansisop-parser/parser

make all

sudo make install
