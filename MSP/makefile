CC = gcc
LIBS = -lcommons -lutiles -lpthread
OBJ = obj/Clock.o obj/LRU.o obj/administradorDeConexionesYConfig.o obj/administradorDeMemoria.o obj/msp.o
DIRS = bin/ obj/ log/
OUT = bin/MSP

all: directories msp

obj/%.o: src/%.c
	$(CC) -c -o $@ $<

msp: $(OBJ)
	$(CC) $(OBJ) $(LIBS) -o $(OUT)

directories:
	mkdir -p $(DIRS)

clean:
	rm -rf $(OUT) $(OBJ) $(DIRS)

.PHONY: directories clean
