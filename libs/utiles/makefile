COMMONS_DIR_HEADERS=../../commons-library/src
COMMONS_DIR_BIN=../../commons-library/src/build
COMMONS_NAME=commons

UTILES=libutiles.so
INSTALL_DIR=/usr/lib

SOURCE_FILES = utiles.c
HEADER_FILER = utiles.h

compile:
	cd utiles; gcc -shared -fPIC $(SOURCE_FILES) $(HEADER_FILES) -I$(COMMONS_DIR_HEADERS) -L$(COMMONS_DIR_BIN) -o $(UTILES) -l$(COMMONS_NAME)
	mv utiles/$(UTILES) .

install: compile
	sudo cp -u $(UTILES) $(INSTALL_DIR)

uninstall:
	sudo rm -f $(INSTALL_DIR)/$(UTILES)
        
clean:
	rm -f $(UTILES)

# PHONY targets son los que no producen un archivo llamado como el target
# Se los marca para evitar confusion si se crea un archivo llamado asi
.PHONY: compile install uninstall clean
