# For MinGW
CC = gcc
# For TCC
# CC = tcc

fatt.exe: fatt.o
	${CC} fatt.o -lkernel32 -o fatt.exe

fatt.o: fatt.c
	${CC} -c fatt.c