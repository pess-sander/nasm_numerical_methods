.PHONY: all clean arc

all: main.o funcs.o main_prog

funcs.o:
	nasm -f elf32 funcs.asm -o funcs.o

# Выбор метода решения уравнения определяется переменной method
# SECANTS - метод секущих
# SEGMENTS - метод деления отрезка пополам (по умолчанию)
main.o:
	gcc -m32 -c main.c -o main.o -D $(method)

main_prog:
	gcc -m32 -lm main.o funcs.o -o main_prog

clean:
	rm *.o main_prog

arc:
	mkdir -p ARC
	zip `date +%Y.%m.%d_%N`.zip main_prog main.c Makefile funcs.asm
	mv *.zip ARC/
