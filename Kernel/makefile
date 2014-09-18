CC = gcc
LIBS = -lcommons -lutiles -lpthread -lansisop-panel
OBJ = obj/kernel.o obj/loader.o obj/planificador.o
DIRS = bin/ obj/ panel/
OUT = bin/Kernel

all: directories kernel

obj/%.o: src/%.c
	$(CC) -c -o $@ $<

kernel: $(OBJ)
	$(CC) $(OBJ) $(LIBS) -o $(OUT)

directories:
	mkdir -p $(DIRS)

clean:
	rm -rf $(OUT) $(OBJ) $(DIRS)

.PHONY: directories clean
