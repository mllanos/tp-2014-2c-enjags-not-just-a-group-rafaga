CC = gcc
IDIR = -I../libs/commons-library/src \
       -I../libs/ansisop-panel \
       -I../libs/utiles 
LDIR = -L../libs/commons-library/src/build \
       -L../libs/ansisop-panel/Release \
       -L../libs/utiles
LIBS = -lcommons -lansisop-panel
OBJ = obj/cpu.o obj/execution_unit.o obj/set_instrucciones.o
DIRS = bin/ obj/ panel/
OUT = bin/CPU

all: directories cpu

obj/%.o: src/%.c
	$(CC) -c $(IDIR) -o $@ $<

cpu: $(OBJ)
	$(CC) $(OBJ) $(LDIR) $(LIBS) -o $(OUT)

directories:
	mkdir -p $(DIRS)

clean:
	rm -rf $(OUT) $(OBJ) $(DIRS)

.PHONY: directories clean
