all: install-libs compile-procs

clean: uninstall-libs clean-procs clean-libs

install-libs:
	cd libs/commons-library && sudo make install
	cd libs/ansisop-panel && sudo make install
	cd libs/utiles && sudo make install

uninstall-libs:
	cd libs/commons-library && sudo make uninstall
	cd libs/ansisop-panel && sudo make uninstall
	cd libs/utiles && sudo make uninstall

compile-procs:
	cd Kernel && make

clean-procs:
	cd Kernel && make clean

clean-libs:
	cd libs/commons-library && make clean
	cd libs/ansisop-panel && make clean
	cd libs/utiles && make clean
