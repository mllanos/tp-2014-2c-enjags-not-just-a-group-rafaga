all: install-libs compile-procs

clean: uninstall-libs clean-procs clean-libs clean-tempfiles

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
	cd CPU && make
	cd MSP && make
	cd Consola && make
	cd Ensamblador && make
	cd Ensamblador && ./massAssemble

clean-procs:
	cd Kernel && make clean
	cd CPU && make clean
	cd MSP && make clean
	cd Consola && make clean
	cd Ensamblador && make clean

clean-libs:
	cd libs/commons-library && sudo make clean
	cd libs/ansisop-panel && sudo make clean
	cd libs/utiles && sudo make clean

clean-tempfiles:
	find . -name "*~" -type f -delete
	find . -name "*Console" -type f -delete
