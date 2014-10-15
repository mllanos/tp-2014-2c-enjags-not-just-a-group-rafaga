#include "msp.h"
#include <stdlib.h>
#include <utiles/utiles.h>
#include <commons/string.h>
#include <commons/collections/node.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>

#define STDIN 0
#define SEG_MAX 1048576
#define PAG_MAX 256
#define LOG_DIR log/MSP.log

typedef struct tipo_segmento{	//Esto representa a un segmento (una fila de la tabla)
	u_int32_t id;
	u_int32_t num_segmento;
	t_list *paginas;
} t_segmento;

typedef struct tipo_pagina{		//Esto representa a una pagina
	u_int32_t num_pagina;
	u_int32_t bit;	//0 , 1 , 2
	u_int32_t bonus;
	u_int32_t num_marco;
} t_pagina;

typedef struct tipo_memoria{		//Esto representa a un marco de memoria principal
	u_int32_t num_marco;
	u_int32_t bit; //0 o 1
} t_memoria;

typedef struct tipo_nodo{
	struct tipo_pagina *pagina;
	struct tipo_segmento *segmento;
} t_nodo;

int puerto, socket_msp, socketKernel; //los que retorna el accept dentro de AtenderConexion
char *point; //Puntero inicial a memoria estatica o principal
char ip[16], cat[5], aux[17], commandAdress[23];
char swap[6] = ".swap";
char *algoritmo;
u_int32_t cmemoria, cswap;
t_log *logfile;
t_list *listaMemoriaPrincipal;
t_list *listaSegmentos; //O lista de segmentos
fd_set descriptoresLectura;

u_int32_t pid;             //Tengo presente el pid y el num segmento del proceso
u_int32_t num_segmento;	// entrante y lo uso por ej para eliminar segmentos...
u_int32_t pagina;

//Espacios en terminos de paginas ;)
u_int32_t pag_mem_total = 0;
u_int32_t pag_tot_mem;
u_int32_t pag_tot_swap;

t_nodo *ultimo;

void borrarSWAPfile(char *adress);
t_log *crearLog(char *archivo);
void cargarConficuracion(char* path);
void crearEstructuras();
void atenderConexion(int sock_msp);
u_int32_t reservarMemoria(u_int32_t pid, u_int32_t tamano);
t_pagina *crearPagina(u_int32_t pagina);
t_memoria *crearMarco(u_int32_t marco);
t_segmento *crearSegmento(u_int32_t id, u_int32_t segmento, t_list *pagina) ;
u_int32_t armarDireccion(u_int32_t segmento, u_int32_t pagina, u_int32_t offset);
void iniciarBasesegmento(u_int32_t *segmento, u_int32_t *pagina, u_int32_t *offset);

bool es_igual_pid(t_segmento *p);
void destruirSegmento(u_int32_t pid,u_int32_t direccion_logica);
void escribirMemoria(u_int32_t pid, u_int32_t direccion_logica, char* bytes,u_int32_t tamano);
u_int32_t calcularRAMocupada();
u_int32_t calcularSWAPocupada(t_list *lista);
u_int32_t espacioTotalOcupado(t_list *lista);
void grabarRAM(char* point, u_int32_t mem_pos, u_int32_t size, char* dato);
void leerRAM(char* point, u_int32_t mem_pos, u_int32_t size, char* dato);
char *crearArchivoSWAP(u_int32_t pid, u_int32_t segmento, u_int32_t pagina);
char *armarSWAPath(u_int32_t pid, u_int32_t segmento, u_int32_t pagina);
void prepararAlgoritmoReemplazo(t_pagina *pagina);
t_memoria *buscarMarcoDisponible(t_list *li);
t_nodo *buscarPaginasCLOCK(t_list *lista, t_nodo *ultimo, t_nodo *elim_swap);
t_nodo *buscarPaginasLRU(t_list *lista, t_nodo *retorno);
t_nodo *buscarReemplazo(t_list *lista, char *string, t_nodo *ultimo);
int escribirSWAPfile(char *adress, char *bytes, u_int32_t tamano);
int  leerSWAPfile(char *adress, char *bytes, int tamano);
void borrarSWAPfile(char *adress);

int main(int argc, char** argv) {


	int select_return, socket_msp;

	//Nos loguemos (utilizando la libreria commons)

	logfile = crearLog(LOG_DIR);

	//Cargamos el archivo de configuracion
	cargarConficuracion(argv[1]);

	crearEstructuras();

	//Creamos el socket de la UMV y esperamos a que el kernel establesca conexion
	socket_msp = crearSocket();
	bindearSocket(socket_msp, ip, puerto);
	escucharSocket(socket_msp);

	while (1) {
		//ESTRUCTURAS PARA SELECT()
		FD_ZERO(&descriptoresLectura);
		FD_SET(socket_msp, &descriptoresLectura);
		FD_SET(STDIN, &descriptoresLectura);			//----->consola

		select_return = select(socket_msp + 1, &descriptoresLectura, NULL, NULL,
				NULL );

		if (select_return < 0) {
			//error
			printf("error en la funcion select **");
			log_error(logfile,
					"main()-Error en el select al esperar por el KERNEL..");
			return -1;
		}
		if (FD_ISSET(0,&descriptoresLectura)) {
			//SE INGRESO UN COMANDO EN LA CONSOLA

		}
		if (FD_ISSET(socket_msp,&descriptoresLectura)) {
			//UNA NUEVA CPU/KERNEL INTENTA CONECTARSE
			atenderConexion(socket_msp);
		}
	}

	return 0;
}

t_log *crearLog(char *archivo) {

	char path[11] = { 0 };
	char aux[17] = { 0 };

	strcpy(path, "logueo.log");
	strcat(aux, "touch ");	//touch, crea un nuevo archivo
	strcat(aux, path);	// strcat, concatena path a aux
	system(aux);	//ejecuta un comando, toma un puntero a char como parametro
	t_log *logAux = log_create(path, archivo, false, LOG_LEVEL_DEBUG); //creo el logger, formato apropiado para usar...
	log_info(logAux, "ARCHIVO DE LOG CREADO");
	return logAux;
}

void cargarConficuracion(char* path) {
	/* el archivo configuracion sera del tipo:
	 * IP=127.0.0.1
	 * PUERTO=5000
	 * CANTIDAD_MEMORIA=KB
	 * CANTIDAD_SWAP=MB
	 * SUST_PAGS=LRU/CLOCK
	 */
	char *ipconfig;
	t_config *configMSP;

	configMSP = config_create(path);//retorna un config con el diccionario de key y datos, mas el path
	ipconfig = config_get_string_value(configMSP, "IP");
	memcpy(ip, ipconfig, strlen(ip) + 1);
	puerto = config_get_int_value(configMSP, "PUERTO");
	cmemoria = 1024 * config_get_int_value(configMSP, "CANTIDAD_MEMORIA");
	cswap = 1048576 * ((u_int32_t)config_get_int_value(configMSP, "CANTIDAD_SWAP"));
	algoritmo = config_get_string_value(configMSP, "SUST_PAGS");
	config_destroy(configMSP);

	//Calculos de espacios

	div_t m = div(cmemoria, PAG_MAX); //Calculo cuantas paginas voy a usar para la memoria y se lo asigno a pag_tot_mem
	pag_tot_mem = m.quot;
	div_t s = div(cswap, PAG_MAX); //Calculo cuantas paginas voy a usar para la swap y se lo asigno a pag_tot_swap
	pag_tot_swap = s.quot;
	pag_mem_total = pag_tot_swap + pag_tot_mem; //actualizo la memoria total

	//printf("archivo de configuracion levantado: ip:%s ...);
	log_debug(logfile,
			"cargarConficuracion(char* path)-Proceso exitoso, con ip:%s puerto:%i tamanio de memoria:%i swap:%i algoritmo:%i ...",
			ip, puerto, cmemoria, cswap, algoritmo);
}

void crearEstructuras() {

	listaSegmentos = list_create();
	point = (char*) malloc(cmemoria);  // Creo memoria principal (malloc).

	listaMemoriaPrincipal = list_create();

	//Creo la lista de marcos de la memoria princippal
	int i ;
	for (i = 0; i < pag_tot_mem; i++) {
		t_memoria *marc = crearMarco(i);
		list_add(listaMemoriaPrincipal, marc);
	}

	log_debug(logfile,
			"crearEstructuras()-Se creo la memoria principal del tp con la direccion fisica %i...",
			(int) (cmemoria));

}

void atenderConexion(int sock_msp) {

	int sockt;
	pthread_t hilo;

	sockt = aceptarConexion(sock_msp);

	log_debug(logfile, "atenderConexion()==>Se detecto una nueva conexion CPU, se lanzara un hilo para que la atienda...");
	pthread_create(&hilo, NULL, &atencionCPK, &sockt);

}

u_int32_t reservarMemoria(u_int32_t pid, u_int32_t tamano) { //Saco id y utilizo el pid que es como variable global
	//acordar usar lista de segmentos para calcular el espacio libre de memoria, vercuales estan en (0)y (1) para saber le de la swap, porque tienen un limite.
	//usar esto al momento de pasar las paginas a memoria y tambien luego al reemplazarlas. para eso me sirve el -1, osea saber en la lista que paginas todavia no se escribieron.
	//el algoritmo de reemplazo influye al reemplazar no ahora, asi que el bonus lo inicio en 0.
	div_t c;
	int c_paginas, i;
	t_list *listaPaginas;

	if (espacioTotalOcupado(listaSegmentos) > pag_mem_total) {

			//error
			log_error(logfile, "crearSegmento()-No se puede crear el segmendo, debido a exceder el valor maximo de memoria disponible, pedido por el proceso: %i ..",pid);
			return -1;
	}

		if (tamano > SEG_MAX) {

			//error
			log_error(logfile,"crearSegmento()-No se puede crear el segmendo, debido a exceder el valor maximo de segmento, pedido por el proceso: %i ..",pid);
			return -1;
			}

		c = div(tamano, PAG_MAX); //Calculo cuantas paginas voy a usar y se lo asigno a c_paginas

			if (c.rem == 0) {
				c_paginas = c.quot;
			}
			else {
				c_paginas = c.quot + 1;
			} //Si la ultima pagina va a estar media vacia. Fragmentacion externa?

		listaPaginas = list_create();

		//Creo las paginas correspondientes

		for(i=0; i < c_paginas; i++){
				t_pagina *pagina;
				pagina = crearPagina(i);
				list_add(listaPaginas, pagina);
			}
		//Anexo la listaPaginas a un segmento que vamos a crear

		t_list *filter_list = list_filter(listaSegmentos, (void*) es_igual_pid);// filtro y me quedo con una lista de pid iguales
		u_int32_t segmentos = list_size(filter_list);

		t_segmento *seg = crearSegmento(pid, segmentos, listaPaginas);

		list_add(listaSegmentos, seg);

		//Elimino la lista temporal
		list_clean(filter_list);
		//Armo la direccion base segmento de todo proceso...
		u_int32_t pagina;
		u_int32_t segmento;
		u_int32_t offset;
		iniciarBasesegmento(&segmento, &pagina, &offset);
		u_int32_t direccion = armarDireccion(pagina, segmento, offset);
		log_debug(logfile,"reservarSegmento()==>Se creo un segmento exitosamente...");


	return direccion;
}

bool es_igual_pid(t_segmento *p) {
return (p->id == pid);
}

bool es_igual_pid_and_seg(t_segmento *p) {
return ((p->id == pid) && (p->num_segmento == num_segmento));
}

bool es_igual_pag(t_pagina *p) {
return ((p->num_pagina) == pagina);
}
/*
 bool pagina_uno(t_segmento *p) { ///Mal pero sirve de ejemplo ->
 return (p->paginas->num_pagina == 1);
 }
 */bool pagina_uno(t_pagina *p) {
return (p->num_pagina == 1);
}

bool pagina_dos(t_pagina *p) {
return (p->num_pagina == 2);
}

bool memoria_ocupada(t_memoria *p) {
return (p->bit == 1);
}

bool memoria_vacia(t_memoria *p) {
return (p->bit == 0);
}

t_segmento *crearSegmento(u_int32_t id, u_int32_t segmento, t_list *pagina) {
t_segmento *new = malloc(sizeof(t_segmento));
new->id = id;
new->num_segmento = segmento;
new->paginas = pagina;

return new;
}

t_pagina *crearPagina(u_int32_t pagina) { //0= reservado 1= memoria 2= swap
t_pagina *new = malloc(sizeof(t_pagina));
new->num_pagina = pagina;
new->bit = 0;
new->bonus = 0;
new->num_marco = 0;

return new;
}

t_memoria *crearMarco(u_int32_t marco) {

t_memoria *new = malloc(sizeof(t_memoria));
new->num_marco = marco;
new->bit = 0; //0 o 1
return new;
}

// Te devuelve un num_byte de un value...
// Byte1 = segmento (12) Byte2 = pagina (12) Byte3 = offset (8)
u_int32_t dameByte(u_int32_t value, int num_byte) {

u_int32_t byte1;
u_int32_t byte2;
u_int32_t byte3;

switch (num_byte) {

case 1:
	byte1 = (value >> 20);
	return byte1;

case 2:
	byte2 = (value >> 8) & 0xfff;
	return byte2;

case 3:
	byte3 = value & 0xff;
	return byte3;

	}
return -1;
}
/* Example value: 0x01020304
 case 1:
 uint32_t byte1 = (value >> 24);           // 0x01020304 >> 24 is 0x01 so
 return byte1;										  // no masking is necessary
 case 2:
 uint32_t byte2 = (value >> 16) & 0xff;    // 0x01020304 >> 16 is 0x0102 so
 return byte2;                                  // we must mask to get 0x02
 case 3:
 uint32_t byte3 = (value >> 8)  & 0xff;    // 0x01020304 >> 8 is 0x010203 so
 return byte3;                                  // we must mask to get 0x03
 case 4:
 uint32_t byte4 = value & 0xff;            // here we only mask, no shifting
 return byte4;                                  // is necessary
 */

u_int32_t armarDireccion(u_int32_t segmento, u_int32_t pagina, u_int32_t offset) {
//Movemos los bits necesarios
u_int32_t seg = (segmento << 20);
u_int32_t pag = (pagina << 8);

//Procedemos a mezclar
u_int32_t aux = (seg | pag);
u_int32_t direccion = (aux | offset);

return direccion;
}

void iniciarBasesegmento(u_int32_t *segmento, u_int32_t *pagina,u_int32_t *offset) {

segmento = 0;
pagina = 0;
offset = 0;
}

void destruirSegmento(u_int32_t pid,u_int32_t direccion_logica) {

	num_segmento = dameByte(direccion_logica, 1);
	t_segmento *delete = list_remove_by_condition(listaSegmentos, (void*) es_igual_pid_and_seg);

	if (delete != NULL ) {

		free(delete);
		log_debug(logfile, "destruirSegmento()==>Se destruyo un segmento exitosamente...");

	} else {
		//error no se encontro segmento, segmentation fault??
		log_error(logfile,"destruirSegmento()-No se puede eliminar el segmendo pedido por el proceso: %i ..",pid);
	}

}

void escribirMemoria(u_int32_t pid, u_int32_t direccion_logica, char* bytes, u_int32_t tamano) {
	char* dato; //para guardar datos en caso de traslado
	t_nodo *outSwap;
	//Consigo el num de segmento
	u_int32_t segmento = dameByte(direccion_logica, 1);
	//Con el pid y el num de segmento busco el nodo correspondiente
	t_segmento *seg = list_find(listaSegmentos, (void*) es_igual_pid_and_seg);
	//Consigo el num de pagina
	u_int32_t pagina = dameByte(direccion_logica, 2);
	//Busco la pagina correspondiente
	t_pagina *pag = list_find(seg->paginas, (void*) es_igual_pag);

	prepararAlgoritmoReemplazo(pag); // Aplico el algoritmo a la pagina

	if (pag != NULL ) {
		log_debug(logfile,
			"escribirMemoria()==>Se encontro la posicion logica en la memoria");
	} else {
		log_error(logfile,
			"escribirMemoria()==>No e encontro la posicion logica en la memoria");
	}

	//Te pido el offset---
	u_int32_t offset = dameByte(direccion_logica, 3);

	if (pag->bit == 0) { //Se reservo pero no se accedio todavia

		t_memoria *memoria = buscarMarcoDisponible(listaMemoriaPrincipal);

	if (memoria != NULL ) {
		//Calculo la direccion exacta para grabar (MARCO GATTI me obligo :( )
		u_int32_t dezplazamiento = 256 * (memoria->num_marco); //Calculo el dezplazamiento "virtual" dentro de la memoria
		char *marco = point + dezplazamiento; // Calculo la direccion exacta a un marco, usando el point (del malloc al principio)
		grabarRAM(marco, offset, tamano, bytes);

		//Actualizo el registro de la estructura principal
		pag->num_marco = memoria->num_marco;
		memoria->bit = 1; //Lamemoria esta ahora ocupada
	} else {
		//Tengo que manejar la swap, implementar algoritmo y desarrollar la busqueda y creacion de archivos swap
		outSwap = buscarReemplazo(listaSegmentos, algoritmo, ultimo);

		outSwap->pagina->bit = 2; //Actualizo el bit a 2, osea en esta en SWAP

		char *adress = crearArchivoSWAP(outSwap->segmento->id, outSwap->segmento->num_segmento, outSwap->pagina->num_pagina);
		//Creo archivo SWAP con los datos encontrados

		//Salvo la informacion en el SWAP file, de la pagina nueva a reemplazar
		leerRAM(point, (outSwap->pagina->num_pagina * 256), 256, dato);
		escribirSWAPfile(adress, dato, 256);

		//Ahora utilizo los datos bytes y tamano, osea para grabar lo q queremos grabar en principio

		u_int32_t dezplazamiento = 256 * (outSwap->pagina->num_marco); //Calculo el dezplazamiento "virtual" dentro de la memoria
		char *marco = point + dezplazamiento; // Calculo la direccion exacta a un marco, usando el point (del malloc al principio)
		grabarRAM(marco, offset, tamano, bytes);

		log_debug(logfile,
				"escribirMemoria(0)==>Se realizo el traslado de memoria a la SWAP");
	}

	pag->bit = 1; // todo salio ok, se grabo el marco
	log_debug(logfile, "escribirMemoria(0)==>Se grabo en la memoria");

} else {

	if (pag->bit == 1) {

		u_int32_t dezplazamiento = 256 * (pag->num_marco); //Calculo el dezplazamiento "virtual" dentro de la memoria
		char *marco = point + dezplazamiento; // Calculo la direccion exacta a un marco, usando el point (del malloc al principio)
		grabarRAM(marco, offset, tamano, bytes);
		log_debug(logfile, "escribirMemoria(1)==>Se grabo en la memoria");
	} else {
		//>>>>>----------Tengo que manejar la swap, es parecido al caso, en u principio, en donde el segmento todavia no se escribio
		outSwap = buscarReemplazo(listaSegmentos, algoritmo,ultimo);

		outSwap->pagina->bit = 2; //Actualizo el bit a 2, osea en esta en SWAP

		char *adress = crearArchivoSWAP(outSwap->segmento->id,
				outSwap->segmento->num_segmento, outSwap->pagina->num_pagina);
		//Creo archivo SWAP con los datos encontrados

		//Salvo la informacion en el SWAP file, de la pagina nueva a reemplazar
		leerRAM(point, (outSwap->pagina->num_pagina * 256), 256, dato);
		escribirSWAPfile(adress, dato, 256);
		//Hasta aca--------------------------------------<<<<<<<<<

		//Ahora debo leer de la SWAP, con la info de la direccion logica
		leerSWAPfile(armarSWAPath(pid, segmento, pagina), dato, 256);
		grabarRAM(point, (outSwap->pagina->num_pagina * 256), 256, dato);
		//Ahora solo queda eliminar esta SWAP vieja
		borrarSWAPfile(armarSWAPath(pid, segmento, pagina));

		//Ahora escribo en la seccion q me indicaron al principio
		grabarRAM(point + (outSwap->pagina->num_pagina * 256), offset, tamano,
				bytes);

	}

}

}

u_int32_t calcularRAMocupada() { //Utilizo la lista secundaria de la RAM || En terminos de numero de paginas

	t_list *ram_list = list_filter(listaMemoriaPrincipal, (void*) memoria_ocupada);
	u_int32_t size = list_size(ram_list);
	list_clean(ram_list);
return size;
}

u_int32_t calcularSWAPocupada(t_list *lista) { //Utilizo la lista principal, utilizando el bit == 2

u_int32_t size = 0;
int i = 0;
while (i <= list_size(lista)) {

	t_segmento *seg = list_get(lista, i);

	int s = 0;
	while (s <= list_size(seg->paginas)) {

		t_pagina *pag = list_get(seg->paginas, s);

		if (pag->bit == 2) {

			size++;

		}
		s++;
	}
	i++;
}
return size;
}

u_int32_t espacioTotalOcupado(t_list *lista) { //|| En terminos de numero de paginas
/*En este caso utilizo la estructura principal, solo cuento todas las paginas que tiene
 ya que no puedo ceder mas paginas de mi limite (swap + ram)*/
	u_int32_t size = 0;
	int i = 0;
	while (i <= list_size(lista)) {

		t_segmento *seg = list_get(lista, i);

		int s = 0;
		while (s <= list_size(seg->paginas)) {

			size++;

			s++;
		}
		i++;
	}
	return size;
	}

void grabarRAM(char* point, u_int32_t mem_pos, u_int32_t size, char* dato) {

	char* pointer = point + mem_pos;

	memcpy(pointer, dato, size);
}

void leerRAM(char* point, u_int32_t mem_pos, u_int32_t size, char* dato) {

	char* pointer = point + mem_pos;

	memcpy(dato, pointer, size);
}

t_memoria *buscarMarcoDisponible(t_list *li) {

	t_memoria *data = list_find(li , (void*) memoria_vacia);
	return data;
}

char *crearArchivoSWAP(u_int32_t pid, u_int32_t segmento, u_int32_t pagina) {
// 6+4+4+3+1+4+|n  //Cantidad total de chars en aux

	strcat(commandAdress, "touch ");	//touch, crea un nuevo archivo

	strcat(commandAdress, armarSWAPath(pid, segmento, pagina));	// strcat, concatena swap a aux

	system(commandAdress);

	return commandAdress;
}

char *armarSWAPath(u_int32_t pid, u_int32_t segmento, u_int32_t pagina) {

	snprintf(cat, 5, "%04d", pid);
	strcat(aux, cat);

	snprintf(cat, 5, "%04d", segmento);
	strcat(aux, cat);

	snprintf(cat, 5, "%03d", pagina);
	strcat(aux, cat);

	strcat(aux, swap);

	return aux;
}

void prepararAlgoritmoReemplazo(t_pagina *pagina) {

if (strcmp(algoritmo, "LRU") == 0) {

	pagina->bonus = pagina->bonus + 1;

}

}
// Esta mal, bueno eso es lo que presiento, asi que lo guardo de todas maneras
/*
 t_pagina buscarPaginasEnRAM(t_list lista){

 t_link_element lista_actual = lista->head;
 t_pagina reemplazo;
 t_link_element aux;

 // recorro cada segmento
 while (lista_actual != NULL){

 while (lista_actual->data->paginas != NULL){
 if (lista_actual->data->bit == 1){

 if (lista_actual>data->bonus == 1){

 reemplazo = lista_actual->data;

 // tengo que guardar el puntero a la posicion siguiente, para la proxima busqueda

 goto encontre;
 }else{
 lista_actual->data->bonus = 1;

 }
 }

 lista_aux = lista_aux->next;
 encontre:
 }

 uint32_t size = list_size(swap_total);
 list_clean(swap_total);
 return size;
 }*/

t_nodo *buscarPaginasCLOCK(t_list *lista, t_nodo *ultimo, t_nodo *elim_swap) {

int i = 0;
while (i <= list_size(lista)) {

	t_segmento *seg = list_get(lista, i);

	int s = 0;
	while (s <= list_size(seg->paginas)) {

		t_pagina *pag = list_get(seg->paginas, s);

		if (pag->bit == 1) {

			if (pag->bonus == 1) {

				ultimo->segmento = seg;
				ultimo->pagina = pag;
				// tengo que guardar el puntero a la posicion siguiente, para la proxima busqueda
				elim_swap->segmento = seg;
				elim_swap->pagina = pag;
				return elim_swap;

			} else {

				pag->bonus = 1;

			}
		}
		s++;
	}
	i++;
}
return elim_swap;
}

t_nodo *buscarPaginasLRU(t_list *lista, t_nodo *retorno) {

u_int32_t maximo = 0;
int i = 0;

while (i <= list_size(lista)) {

	t_segmento *seg = list_get(lista, i);
	int s = 0;
	while (s <= list_size(seg->paginas)) {

		t_pagina *pag = list_get(seg->paginas, s);

		if (pag->bit == 1) {

			if (maximo == 0)
				maximo = pag->bonus;

			if (pag->bonus < maximo) {

				//Me guardo el valor mas chico, ya que es el menos usado, lo saco-
				maximo = pag->bonus;
				retorno->pagina = pag;
				retorno->segmento = seg;
				//Y luego guardo la pagina para retornar-
			}

		}
		s++;
	}
	i++;
}
return retorno;

}

t_nodo *buscarReemplazo(t_list *lista, char *string, t_nodo *ultimo) {
	t_nodo *retorno;
	if (strcmp(string,"LRU") == 0) {
		 buscarPaginasLRU(lista, retorno);
		 return retorno;
	}

	buscarPaginasCLOCK(lista, ultimo, retorno);
	return retorno;

}

int escribirSWAPfile(char *adress, char *bytes, u_int32_t tamano) {

FILE *ptr;

ptr = fopen(adress, "w");
if (!ptr) {
	printf("Unable to open file!");
	return -1;
}

fwrite(bytes, tamano, 1, ptr);

fclose(ptr);
return 0;
}

int leerSWAPfile(char *adress, char *bytes, int tamano) {

	FILE *ptr;

	ptr = fopen(adress, "r");
	if (!ptr) {
		printf("Unable to open file!");
		return -1;
		}

	fread(bytes, tamano, 1, ptr);

	fclose(ptr);
	return 0;
}

void borrarSWAPfile(char *adress) {

	remove(adress);
}
// en el puntero bytes te retorna el resultado

int solicitarMemoria(u_int32_t pid, u_int32_t direccion_logica, char* bytes, u_int32_t tamano) {

	//Consigo el num de segmento
	u_int32_t segmento = dameByte(direccion_logica, 1);
	//Con el pid y el num de segmento busco el nodo correspondiente
	t_segmento *seg = list_find(listaSegmentos, (void*) es_igual_pid_and_seg);
	//Consigo el num de pagina
	u_int32_t pagina = dameByte(direccion_logica, 2);
	//Busco la pagina correspondiente
	t_pagina *pag = list_find(seg->paginas, (void*) es_igual_pag);

	prepararAlgoritmoReemplazo(pag); // Aplico el algoritmo a la pagina

	if (pag != NULL ) {
		log_debug(logfile,
			"escribirMemoria()==>Se encontro la posicion logica en la memoria");
		} else {
		log_error(logfile,
			"escribirMemoria()==>No e encontro la posicion logica en la memoria");
		}

	//Te pido el offset---
	u_int32_t offset = dameByte(direccion_logica, 3);

	if (pag->bit == 0) { //Se reservo pero no se accedio todavia

		log_error(logfile,"solicitarMemoria(0)==>La memoria solamente se reservo, segmentation fault?");
		return -1;
		}

	if (pag->bit == 1) {

		u_int32_t dezplazamiento = 256 * (pag->num_marco); //Calculo el dezplazamiento "virtual" dentro de la memoria
		char *marco = point + dezplazamiento; // Calculo la direccion exacta a un marco, usando el point (del malloc al principio)
		leerRAM(marco, offset, tamano, bytes);
		log_debug(logfile, "escribirMemoria(1)==>Se grabo en la memoria");

	}
	if (pag->bit == 2) {
		//>>>>>----------Tengo que manejar la swap, es parecido al caso, en u principio, en donde el segmento todavia no se escribio
		t_nodo *outSwap = buscarReemplazo(listaSegmentos, algoritmo,ultimo);

		outSwap->pagina->bit = 2; //Actualizo el bit a 2, osea en esta en SWAP

		char *adress = crearArchivoSWAP(outSwap->segmento->id, outSwap->segmento->num_segmento, outSwap->pagina->num_pagina);
		//Creo archivo SWAP con los datos encontrados

		//Salvo la informacion en el SWAP file, de la pagina nueva a reemplazar
		char* dato;
		leerRAM(point, (outSwap->pagina->num_pagina * 256), 256, dato);
		escribirSWAPfile(adress, dato, 256);
		//Hasta aca--------------------------------------<<<<<<<<<

		//Ahora debo leer de la SWAP, con la info de la direccion logica
		leerSWAPfile(armarSWAPath(pid, segmento, pagina), dato, 256);
		grabarRAM(point, (outSwap->pagina->num_pagina * 256), 256, dato);
		//Ahora solo queda eliminar esta SWAP vieja
		borrarSWAPfile(armarSWAPath(pid, segmento, pagina));

		//Ahora escribo en la seccion q me indicaron al principio
		leerRAM((point + (outSwap->pagina->num_pagina * 256)), offset, tamano, bytes);

		}


	return 0;
}

void atencionCPK(void *sock_fd){ //Notar que en la creacion de mensaje, el id es el pid, creo que lo debo corregir luego, dependiendo lo que ustedes deseen recibir...

while(1){

	t_msg *msg = recibir_mensaje(sock_fd);

	u_int32_t id;
	u_int32_t size;
	u_int32_t direccion;
	char *datos;

	log_debug(logfile,"atencionCPK()==>Se recibio el msg");

	switch(msg->header){

		case RESERVE_SEGMENT:

				memcpy(&id,msg->stream,REG_SIZE);
				memcpy(&size, msg->stream + REG_SIZE,REG_SIZE);

				direccion = reservarMemoria(id, size);

				int stream_size = 2*REG_SIZE ;
				char *stream = malloc(stream_size);
				memcpy(stream,&pid,REG_SIZE);
				memcpy(stream + REG_SIZE,&direccion,REG_SIZE);

						t_msg *new_msg = crear_mensaje(id,stream,stream_size);

						enviar_mensaje(sock_fd,new_msg);
						destroy_message(new_msg);

				break;

		case DESTROY_SEGMENT:

			    memcpy(&id,msg->stream,REG_SIZE);
				memcpy(&direccion,msg->stream + REG_SIZE,REG_SIZE);

				destruirSegmento(id, direccion);

				break;
		case MEM_REQUEST:

				memcpy(&id,msg->stream,REG_SIZE);
				memcpy(&direccion,msg->stream + REG_SIZE,REG_SIZE);
				memcpy(&size,msg->stream + 2*REG_SIZE,sizeof size);

				if (solicitarMemoria(id, direccion, datos, size) < 0) {
					//error
				}else{
						int stream_size = REG_SIZE + sizeof size;
						char *stream = malloc(stream_size);
						memcpy(stream,&pid,REG_SIZE);
						memcpy(stream + REG_SIZE,datos,sizeof size);

						t_msg *new_msg = crear_mensaje(id,stream,stream_size);

						enviar_mensaje(sock_fd,new_msg);
						destroy_message(new_msg);

				}

				break;

		case WRITE_MEMORY:

				memcpy(&id,msg->stream,REG_SIZE);
				memcpy(&direccion,msg->stream + REG_SIZE,REG_SIZE);
				memcpy(&size,msg->stream + 2*REG_SIZE,sizeof size);
				memcpy(&datos,msg->stream + 2*REG_SIZE + sizeof size,size);

				escribirMemoria(id, direccion, datos, size);

				//error
				break;
	}
	destroy_message(msg);
	}
}

void consolaMSP () {
	char command[35];
	char action[15];
	char stream[10];
	char *texto;
	u_int32_t pid;
	u_int32_t size;
	u_int32_t direccion;
	u_int32_t direccionVirtual;
	while(1){
		scanf("%s",command);
		int i=0;
		while(command[i]!="" || command[i]!="\0"){
			action[i]=command[i];
			i++;
		}
		int j=i+1;
		i=0;


		if (strcmp(action,"createSegment",i)==0) { //CREAR_SEGMENTO
			while(command[j]!=",") {
				stream[i]=command[j];
				i++;
				j++;
			}
			j++;
			i=0;
			pid=atoi(stream); //capturo el pid
			stream[0]="\0";
			while(command[j]!="\0") {
				stream[i]=command[j];
				i++;
				j++;
			}
			size=atoi(stream);
			direccion = reservarMemoria(pid, size); // capturo direcionBase
			printf(direccion); //imprimo por pantalla y en el log
			log_trace(logfile,"Direccion Base:%n",direccion);
		}


		else if (strcmp(action,"destroySegment",i)==0) { //DESTRUIR_SEGMENTO
			while(command[j]!=",") {
				stream[i]=command[j];
				i++;
				j++;
			}
			j++;
			i=0;
			pid=atoi(stream); // capturo pid
			stream[0]="\0";
			while(command[j]!="\0") {
				stream[i]=command[j];
				i++;
				j++;
			}
			direccion=atoi(stream); // capturo direccion
			destruirSegmento(pid,direccion); // destruccion de segmento
		}


		else if (strcmp(action,"memoryWrite",i)==0) { //ESCRIBIR_MEMORIA
			while(command[j]!=",") {
				stream[i]=command[j];
				i++;
				j++;
			}
				j++;
				i=0;
				pid=atoi(stream); // capturo pid
				stream[0]="\0";
			while(command[j]!=",") {
				stream[i]=command[j];
				i++;
				j++;
			}
				direccionVirtual=atoi(stream);
				j++;
				i=0;
				stream[0]="\0";
			while(command[j]!=",") {
				stream[i]=command[j];
				i++;
				j++;
			}
				size=atoi(stream);
				j++;
				i=0;
				stream[0]="\0";
			while(command[j]!="\0") {
				stream[i]=command[j];
				i++;
				j++;
			}
				texto=stream;
				escribirMemoria(pid,direccion,texto,size);
		}


		else if (strcmp(action,"memoryRead",i)==0) {
			while(command[j]!=",") {
				stream[i]=command[j];
				i++;
				j++;
			}
				j++;
				i=0;
				pid=atoi(stream); // capturo pid
				stream[0]="\0";
			while(command[j]!=",") {
				stream[i]=command[j];
				i++;
				j++;
			}
				direccionVirtual=atoi(stream);
				j++;
				i=0;
				stream[0]="\0";
			while(command[j]!="\0") {
				stream[i]=command[j];
				i++;
				j++;
			}
				size=atoi(stream);
				//falta leerMemoria
		}


		else if (strcmp(action,"segmentsTable",i)==0) {
			t_link_element *element = listaMemoriaPrincipal->head;
			while (element != NULL) {
				printf(element->data);
				// faltan los sizes
				element = element->next;
			}
		}


		else if (strcmp(action,"pagesTable",i)==0) {
			while(command[j]!=",") {
				stream[i]=command[j];
				i++;
				j++;
			}
				pid=atoi(stream);
				//imprimir tabla de paginas
		}


		else if (strcmp(action,"listFrames",i)==0) {
			t_link_element *element = listaMemoriaPrincipal->head;
			while (element != NULL) {
				printf(element->data);
				// falta sacar el pid
				element = element->next;
			}
		}

		else {
			printf("Error de comando");
			log_trace(logfile,"Error de comando");
		}

	}
}


