CFLAGS := -I../libs/commons-library

LDFLAGS := -L../libs/commons-library

LIB := -lcommons

all: main.c
	gcc $(CFLAGS) $(LDFLAGS) main.c $(LIB) -o Kernel

clean:
	touch Kernel && $(RM) Kernel
