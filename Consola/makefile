CC = gcc
LIBS = -lcommons -lutiles
OBJ = obj/consola.o 
DIRS = bin/ obj/ log/ BESO/
OUT = bin/Consola

all: directories consola

obj/%.o: src/%.c
	$(CC) -c -o $@ $<

consola: $(OBJ)
	$(CC) $(OBJ) $(LIBS) -o $(OUT)

directories:
	mkdir -p $(DIRS)

clean:
	rm -rf $(OUT) $(OBJ) $(DIRS)

.PHONY: directories clean
