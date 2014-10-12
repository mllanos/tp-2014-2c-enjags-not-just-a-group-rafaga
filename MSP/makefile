CC = gcc
LIBS = -lcommons -lutiles -lansisop-panel
OBJ = obj/msp.o
DIRS = bin/ obj/
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
