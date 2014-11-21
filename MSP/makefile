CC = gcc
LIBS = -lcommons -lutiles -lpthread
OBJ = obj/Clock.o obj/LRU.o obj/administradorDeConexionesYConfig.o obj/administradorDeMemoria.o obj/msp.o
DIRS = bin/ obj/ log/ swap/
OUT = bin/MSP
FLAGS = -g -Wall

all: directories msp

obj/%.o: src/%.c
	$(CC) $(FLAGS) -c -o $@ $<

msp: $(OBJ)
	$(CC) $(FLAGS) $(OBJ) $(LIBS) -o $(OUT)

directories:
	mkdir -p $(DIRS)

clean:
	rm -rf $(OUT) $(OBJ) $(DIRS)

.PHONY: directories clean
