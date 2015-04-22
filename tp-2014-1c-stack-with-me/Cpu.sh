#!/bin/bash

clear

ubicacion=$HOME/tp-2014-1c-stack-with-me

# Compilando el Proceso Kernel

cd $ubicacion/CPU/src

gcc -o ProcesoCPU cpu.c ejecucion.c funcionesCpu.c funcionesPrimitivas.c kernel.c manejoKernel.c manejoStack.c -lcommons -lpthread -lparser-ansisop

# mv ProcesoCPU $ubicacion/CPU

# Ejecutando el Proceso Kernel

# cd ..

clear

./ProcesoCPU


