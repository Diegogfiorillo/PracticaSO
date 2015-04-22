#!/bin/bash

clear

ubicacion=$HOME/tp-2014-1c-stack-with-me

# Compilando el Proceso Kernel

cd $ubicacion/Programa

gcc -o ProcesoPrograma programa.c funcionesPrograma.c -lcommons

# Ejecutando el Proceso Kernel

# cd ..

clear

echo "¿Cual script desea ejecutar?" 

read script

clear

./$script
