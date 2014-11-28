INSTALL_DIR=/usr/lib
LIB_DIR=/usr/include
ANSISOP=libansisop-panel.so
C_SRCS=$(shell find . -iname "*.c" | tr '\n' ' ')
H_SRCS=$(shell find . -iname "*.h" | tr '\n' ' ')

compile:
	gcc -shared -fPIC $(C_SRCS) $(H_SRCS) -o $(ANSISOP) -lcommons

install: compile
	cp -u $(ANSISOP) $(INSTALL_DIR)
	cp --parents -u $(H_SRCS) $(LIB_DIR)

uninstall: clean
	rm -f $(INSTALL_DIR)/$(ANSISOP)
	rm -rf $(LIB_DIR)/panel

clean:
	rm -f $(ANSISOP)



