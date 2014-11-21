CC = gcc
LIBS = -lcommons -lutiles
OBJ = obj/consola.o 
DIRS = bin/ obj/ log/ BESO/
OUT = bin/Consola
FLAGS = -g -Wall

all: directories consola

obj/%.o: src/%.c
	$(CC) $(FLAGS) -c -o $@ $<

consola: $(OBJ)
	$(CC) $(FLAGS) $(OBJ) $(LIBS) -o $(OUT)

directories:
	mkdir -p $(DIRS)

clean:
	rm -rf $(OUT) $(OBJ) $(DIRS)

.PHONY: directories clean
