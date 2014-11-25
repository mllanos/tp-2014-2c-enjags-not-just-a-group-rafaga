CC = gcc
LIBS = -lcommons -lutiles -lpthread -lansisop-panel
OBJ = obj/kernel.o obj/loader.o obj/planificador.o
DIRS = bin/ obj/ panel/ log/
OUT = bin/Kernel
FLAGS = -g -Wall

all: directories kernel

obj/%.o: src/%.c
	$(CC) $(FLAGS) -c -o $@ $<

kernel: $(OBJ)
	$(CC) $(FLAGS) $(OBJ) $(LIBS) -o $(OUT)

directories:
	mkdir -p $(DIRS)

clean:
	rm -rf $(OUT) $(OBJ) $(DIRS)

.PHONY: directories kernel clean
