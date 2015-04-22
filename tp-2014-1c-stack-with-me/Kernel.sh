
#!/bin/bash

clear

ubicacion=$HOME/git/tp-2014-1c-stack-with-me

# Compilando el Proceso Kernel

cd $ubicacion/Proceso\ Kernel/src

gcc -o ProcesoKernel Kernel.c FuncionesPLP.c FuncionesPCP.c -lcommons -lpthread -lparser-ansisop

mv ProcesoKernel $ubicacion/Proceso\ Kernel

# Ejecutando el Proceso Kernel

cd ..

clear

./ProcesoKernel
