#ifndef ERROR_H_
#define ERROR_H_

	#include <commons/log.h>
	#include "console.h"

	void errorHilo(int error);
	void errorPath();
	void errorConexion();
	void errorNivel(char* nivel);
	void errorAlgoritmo(char* algoritmo);
	void errorMensaje(char* mensaje);

	t_log* logger;

#endif /* ERROR_H_ */
