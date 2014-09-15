CC = gcc
IDIR = -I../libs/commons-library/src \
       -I../libs/ansisop-panel \
       -I../libs/utiles
LDIR = -L../libs/commons-library/src/build \
       -L../libs/ansisop-panel/Release \
       -L../libs/utiles
LIBS = -lcommons -lutiles -lpthread -lansisop-panel
OBJ = obj/kernel.o obj/loader.o obj/planificador.o
DIRS = bin/ obj/ panel/
OUT = bin/Kernel

all: directories kernel

obj/%.o: src/%.c
	$(CC) -c $(IDIR) -o $@ $<

kernel: $(OBJ)
	$(CC) $(OBJ) $(LDIR) $(LIBS) -o $(OUT)

directories:
	mkdir -p $(DIRS)

clean:
	rm -rf $(OUT) $(OBJ) $(DIRS)

.PHONY: directories clean
