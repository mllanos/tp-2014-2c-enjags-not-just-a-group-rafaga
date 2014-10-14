CC = gcc
LIBS = -lcommons -lutiles
OBJ = obj/msp.o
DIRS = bin/ obj/
OUT = bin/MSP

all: directories MSP

obj/%.o: src/%.c
	$(CC) -c -o $@ $<

MSP: $(OBJ)
	$(CC) $(OBJ) $(LIBS) -o $(OUT)

directories:
	mkdir -p $(DIRS)

clean:
	rm -rf $(OUT) $(OBJ) $(DIRS)

.PHONY: directories clean
