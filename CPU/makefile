CC = gcc
LIBS = -lcommons -lutiles -lansisop-panel
OBJ = obj/cpu.o obj/execution_unit.o obj/set_instrucciones.o
DIRS = bin/ obj/
OUT = bin/CPU
FLAGS = -g

all: directories cpu

obj/%.o: src/%.c
	$(CC) $(FLAGS) -c -o $@ $<

cpu: $(OBJ)
	$(CC) $(FLAGS) $(OBJ) $(LIBS) -o $(OUT)

directories:
	mkdir -p $(DIRS)

clean:
	rm -rf $(OUT) $(OBJ) $(DIRS)

.PHONY: directories clean
