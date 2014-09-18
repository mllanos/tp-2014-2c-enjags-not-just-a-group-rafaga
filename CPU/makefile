CC = gcc
IDIR = -I../libs/commons-library/src \
       #-I../libs/ansisop-panel \
       #-I../libs/utiles
LDIR = -L../libs/commons-library/src/build \
       #-L../libs/ansisop-panel/Release \
       #-L../libs/utiles
LIBS = -lcommons -lpthread #-lansisop-panel #-lutiles 
OBJ = Debug/obj/cpu.o Debug/obj/execution_unit.o Debug/obj/set_instrucciones.o
DIRS = Debug/obj/ Debug/panel/
OUT = cpu

all: directories cpu

Debug/obj/%.o: src/%.c
	$(CC) -c $(IDIR) -o $@ $<

cpu: $(OBJ)
	$(CC) $(OBJ) $(LDIR) $(LIBS) -o $(OUT)

directories:
	mkdir -p $(DIRS)

clean:
	rm -rf $(OUT) $(OBJ) $(DIRS)

.PHONY: directories clean
