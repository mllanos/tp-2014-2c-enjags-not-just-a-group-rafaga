CC = gcc
LIBS = -lcommons -lutiles -lpthread
OBJ = main.o
OUT = MSP

all: main

%.o: %.c
	$(CC) -c -o $@ $<

main: $(OBJ)
	$(CC) $(OBJ) $(LIBS) -o $(OUT)

clean:
	rm -rf $(OUT) $(OBJ)

.PHONY: clean
