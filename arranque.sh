#!/bin/bash
# -*- ENCODING: UTF-8 -*-
cd /home/utnso
mkdir kernel
cd /home/utnso/git
git clone https://github.com/sisoputnfrba/so-commons-library.git
git clone https://github.com/sisoputnfrba/so-ensamblador.git
git clone https://github.com/sisoputnfrba/ansisop-panel.git
git clone https://github.com/sisoputnfrba/scripts-eso.git
cd /home/utnso/git/so-commons-library
make install
cd /home/utnso/git/so-ensamblador
make all
cp systemCalls.txt /home/utnso/git/so-ensamblador/build
cd /home/utnso/git/so-ensamblador/EjemplosESO
cp arithmetics.txt /home/utnso/git/so-ensamblador/build
cp bigStack.txt /home/utnso/git/so-ensamblador/build
cp cpyloop.txt /home/utnso/git/so-ensamblador/build
cp data.txt /home/utnso/git/so-ensamblador/build
cp flow.txt /home/utnso/git/so-ensamblador/build
cp getMemory.txt /home/utnso/git/so-ensamblador/build
cp labels.txt /home/utnso/git/so-ensamblador/build
cp load.txt /home/utnso/git/so-ensamblador/build
cp logic.txt /home/utnso/git/so-ensamblador/build
cp segFault.txt /home/utnso/git/so-ensamblador/build
cp shifter.txt /home/utnso/git/so-ensamblador/build
cp stack.txt /home/utnso/git/so-ensamblador/build
cd /home/utnso/git/so-ensamblador/EjemplosESO/systemCalls
cp free.txt /home/utnso/git/so-ensamblador/build
cp hilos.txt /home/utnso/git/so-ensamblador/build
cp malloc.txt /home/utnso/git/so-ensamblador/build
cp semaphores.txt /home/utnso/git/so-ensamblador/build
cp STDIN.txt /home/utnso/git/so-ensamblador/build
cp STDOUT.txt /home/utnso/git/so-ensamblador/build
cd /home/utnso/git/scripts-eso
cp prueba_error.txt /home/utnso/git/so-ensamblador/build
cp prueba_hilos.txt /home/utnso/git/so-ensamblador/build
cp prueba_suma.txt /home/utnso/git/so-ensamblador/build
cp prueba_suma_alocada.txt /home/utnso/git/so-ensamblador/build
cp prueba_forES.txt /home/utnso/git/so-ensamblador/build
cd /home/utnso/git/scripts-eso/prueba_prodcons
cp prueba_consumidor.txt /home/utnso/git/so-ensamblador/build
cp prueba_productor.txt /home/utnso/git/so-ensamblador/build
cd /home/utnso/git/so-ensamblador/build
./so-ensamblador -v -o systemCalls.bc systemCalls.txt
rm systemCalls.txt
cp systemCalls.bc /home/utnso/kernel
./so-ensamblador -v -o arithmetics.bc arithmetics.txt
rm arithmetics.txt
./so-ensamblador -v -o bigStack.bc bigStack.txt
rm bigStack.txt
./so-ensamblador -v -o cpyloop.bc cpyloop.txt
rm cpyloop.txt
./so-ensamblador -v -o data.bc data.txt
rm data.txt
./so-ensamblador -v -o flow.bc flow.txt
rm flow.txt
./so-ensamblador -v -o getMemory.bc getMemory.txt
rm getMemory.txt
./so-ensamblador -v -o labels.bc labels.txt
rm labels.txt
./so-ensamblador -v -o load.bc load.txt
rm load.txt
./so-ensamblador -v -o logic.bc logic.txt
rm logic.txt
./so-ensamblador -v -o segFault.bc segFault.txt
rm segFault.txt
./so-ensamblador -v -o shifter.bc shifter.txt
rm shifter.txt
./so-ensamblador -v -o stack.bc stack.txt
rm stack.txt
./so-ensamblador -v -o free.bc free.txt
rm free.txt
./so-ensamblador -v -o hilos.bc hilos.txt
rm hilos.txt
./so-ensamblador -v -o malloc.bc malloc.txt
rm malloc.txt
./so-ensamblador -v -o semaphores.bc semaphores.txt
rm semaphores.txt
./so-ensamblador -v -o stdin.bc STDIN.txt
rm STDIN.txt
./so-ensamblador -v -o stdout.bc STDOUT.txt
rm STDOUT.txt
./so-ensamblador -v -o prueba_error.bc prueba_error.txt
rm prueba_error.txt
./so-ensamblador -v -o prueba_forES.bc prueba_forES.txt
rm prueba_forES.txt
./so-ensamblador -v -o prueba_hilos.bc prueba_hilos.txt
rm prueba_hilos.txt
./so-ensamblador -v -o prueba_suma.bc prueba_suma.txt
rm prueba_suma.txt
./so-ensamblador -v -o prueba_suma_alocada.bc prueba_suma_alocada.txt
rm prueba_suma_alocada.txt
./so-ensamblador -v -o prueba_consumidor.bc prueba_consumidor.txt
rm prueba_consumidor.txt
./so-ensamblador -v -o prueba_productor.bc prueba_productor.txt
rm prueba_productor.txt
cd /home/utnso/git/tp-2014-2c-rafagadeterror/tpAlternativo/consola/src
gcc -o consola cs.c -lcommons -lpthread
cd /home/utnso/git/tp-2014-2c-rafagadeterror/tpAlternativo/kernel/src
gcc -o kernel kernel.c -lcommons -lpthread
cd /home/utnso/git/tp-2014-2c-rafagadeterror/tpAlternativo/cpu/src
gcc -o cpu c.c -lcommons -lpthread
cd /home/utnso/git/tp-2014-2c-rafagadeterror/tp/msp/src
gcc -o msp msp.c -lcommons -lpthread
echo "Se compilo todo correctamente"
exit
