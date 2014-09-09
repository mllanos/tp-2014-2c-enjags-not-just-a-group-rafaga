INSTALL_DIR=/usr/lib
ANSISOP=libansisop-panel.so

compile:
	cd Release && $(MAKE)

install: compile
	sudo cp -u Release/$(ANSISOP) $(INSTALL_DIR)

clean:
	cd Release && $(MAKE) clean

uninstall:
	$(RM) $(INSTALL_DIR)/$(ANSISOP)
