#!/bin/bash

clear

ubicacion=$HOME/git/tp-2014-1c-stack-with-me

# Compilando el Proceso Kernel

cd $ubicacion/UMV/src

gcc -o ProcesoUMV umv.c funcionesConsola.c funcionesCPU.c funcionesUMV.c gestionDeSegmentos.c serials.c -lcommons -lpthread

mv ProcesoUMV $ubicacion/UMV

# Ejecutando el Proceso Kernel

cd ..

clear

./ProcesoUMV
