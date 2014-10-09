
#include "umv.h"
#include <stdlib.h>

#define STDIN            0
#define SEG_MAX			 1048576
#define PAG_MAX          256


int  puerto, socketHilo,socketKernel; //los que retorna el accept dentro de AtenderConexion
char *point; //Puntero inicial a memoria estatica o principal
char ip[16], algoritmo[8]; cat[5];aux[17];adress[23];
char swap[6]= ".swap";
u_int32_t cmemoria, cswap;
t_log  *logfile;
t_list *listaMemoriaPrincipal;
t_list *listaSegmentos; //O lista de segmentos
fd_set  descriptoresLectura;

u_int32_t pid;                  //Tengo presente el pid y el num segmento del proceso
u_int32_t num_segmento;			// entrante y lo uso por ej para eliminar segmentos...


//Espacios en terminos de paginas ;)
u_int32_t pag_mem_total = 0;
u_int32_t pag_tot_mem;
u_int32_t pag_tot_swap;

typedef struct {		//Esto representa a un segmento (una fila de la tabla)
	u_int32_t id;
	u_int32_t num_segmento;
	t_lista *paginas;
} t_segmento;

typedef struct {		//Esto representa a una pagina
	u_int32_t num_pagina;
	u_int32_t bit;	//0 , 1 , 2
	u_int32_t bonus;
	u_int32_t num_marco;
} t_pagina;

typedef struct {		//Esto representa a un marco de memoria principal
	u_int32_t num_marco;
	u_int32_t bit; //0 o 1
} t_memoria;

int main (int argc, char** argv){

	int select_return, socket_msp;

	//Nos loguemos (utilizando la libreria commons)
	loguer=crearLog(argv[0]);

	//Cargamos el archivo de configuracion
	cargarConficuracion(argv[1]);

	crearEstructuras();

	//Creamos el socket de la UMV y esperamos a que el kernel establesca conexion
	socket_msp = crearSocket();
	bindearSocket(socket_msp,ip,puerto);
	escucharSocket(socket_msp);

	while(1){
			//ESTRUCTURAS PARA SELECT()
			FD_ZERO(&descriptoresLectura);
			FD_SET(socket_msp,&descriptoresLectura);
			FD_SET(STDIN,&descriptoresLectura);//----->consola

			select_return = select(socket_msp+1,&descriptoresLectura,NULL,NULL,NULL);

				if(select_return<0){
					//error
					printf("error en la funcion select **");
					log_error(logfile,"main()-Error en el select al esperar por el KERNEL..");
					return -1;
				}
				if(FD_ISSET(0,&descriptoresLectura)){
					//SE INGRESO UN COMANDO EN LA CONSOLA

				}
				if(FD_ISSET(socket_umv,&descriptoresLectura)){
					//UNA NUEVA CPU/KERNEL INTENTA CONECTARSE
					atenderConexion(socket_msp);
				}
			}


	return 0;
}

t_log *crearLog(char *archivo){

	char path[11]={0};
	char aux[17]={0};

	strcpy(path,"logueo.log");
	strcat(aux,"touch ");	//touch, crea un nuevo archivo
	strcat(aux,path);	// strcat, concatena path a aux
	system(aux);	//ejecuta un comando, toma un puntero a char como parametro
	t_log *logAux=log_create(path,archivo,false,LOG_LEVEL_DEBUG); //creo el logger, formato apropiado para usar...
	log_info(logAux,"ARCHIVO DE LOG CREADO");
	return logAux;
}



void cargarConficuracion(char* path){
	/* el archivo configuracion sera del tipo:
	 * IP=127.0.0.1
	 * PUERTO=5000
	 * CANTIDAD_MEMORIA=KB
	 * CANTIDAD_SWAP=MB
	 * SUST_PAGS=LRU/CLOCK
	 */
	char          *ipconfig;
	t_config      *configUMV;
	extern t_log  *loguer;

	configUMV=config_create(path);	//retorna un config con el diccionario de key y datos, mas el path
	ipconfig      =config_get_string_value(configUMV,"IP");
	memcpy(ip,ipconfig,strlen(ip)+1);
	puerto 		  =config_get_int_value(configUMV,"PUERTO");
	cmemoria      =1024 * config_get_int_value(configUMV,"CANTIDAD_MEMORIA");
	cswap         =1048576 * config_get_int_value(configUMV,"CANTIDAD_SWAP");
	algoritmo     =config_get_string_value(configUMV,"SUST_PAGS");
	config_destroy(configUMV);

	//Calculos de espacios

	div_t m = div(cmemoria, PAG_MAX); //Calculo cuantas paginas voy a usar para la memoria y se lo asigno a pag_tot_mem
	pag_tot_mem = m.quot;
	div_t s = div(cswap, PAG_MAX); //Calculo cuantas paginas voy a usar para la swap y se lo asigno a pag_tot_swap
	pag_tot_swap = s.quot;
	mem_total = cmemoria + cswap; //actualizo la memoria total

	//printf("archivo de configuracion levantado: ip:%s ...);
	log_debug(logfile,"cargarConficuracion(char* path)-Proceso exitoso, con ip:%s puerto:%i tamanio de memoria:%i swap:%i algoritmo:%i ...",ip,puerto,cmemoria,cswap,algoritmo);
}

void crearEstructuras(){

	listaSegmentos=list_create();
	point = (char*) malloc(cmemoria);  // Creo memoria principal (malloc).

	listaMemoriaPrincipal=list_create();

	//Creo la lista de marcos de la memoria princippal

	for(int i=0; i < pag_tot_mem; i++)
				{
					t_marco *marc = crearMarco(i);
					list_add(listaMemoriaPrincipal, marc);
				}

	log_debug(logfile,"crearEstructuras()-Se creo la memoria principal del tp con la direccion fisica %i...",(int)(cmemoria));

}

void atenderConexion(int sock_msp){

	int           sockt;
	pthread_t     hiloKernel;
	pthread_t     hiloCPU;
	t_msg         mensaje;
	mensaje.flujoDatos=NULL;

	sockt=aceptarConexion(sock_msp);
	recibirMsg(sockt,&mensaje);

	if(mensaje.encabezado.codMsg==KERNEL){
		socketKernel=sockt;
		log_debug(logfile,"atenderConexion()==>Se detecto una nueva conexion KERNEL, se lanzara un hilo para que la atienda...");
		pthread_create(&idHiloKernel,NULL,&hiloAtencionKernel,NULL);
	}
	if(mensaje.encabezado.codMsg==CPU){
		socketHilo=sockt;
		log_debug(logfile,"atenderConexion()==>Se detecto una nueva conexion CPU, se lanzara un hilo para que la atienda...");
		pthread_create(&threadHilo,NULL,&hiloAtencionCPU,&socketHilo);
	}
}

void reservarMemoria(int tamaño) { //Saco id y utilizo el pid que es como variable global
	//acordar usar lista de segmentos para calcular el espacio libre de memoria, vercuales estan en (0)y (1) para saber le de la swap, porque tienen un limite.
	//usar esto al momento de pasar las paginas a memoria y tambien luego al reemplazarlas. para eso me sirve el -1, osea saber en la lista que paginas todavia no se escribieron.
	//el algoritmo de reemplazo influye al reemplazar no ahora, asi que el bonus lo inicio en 0.

	uint32_t c_paginas;
	if (espacioTotalOcupado() < pag_mem_total){


		if tamaño < SEG_MAX{

			div_t c = div(tamaño, PAG_MAX); //Calculo cuantas paginas voy a usar y se lo asigno a c_paginas

			if (c.rem == 0) {c_paginas = c.quot;} else {c_paginas = c.quot + 1;} //Si la ultima pagina va a estar media vacia. Fragmentacion externa?

			t_list listaPaginas = list_create();

			//Creo las paginas correspondientes
			for(int i=0; i < c_paginas; i++)
				{
				t_segmento *pagina = crearPagina(i);
				list_add(listaPaginas, pagina);
				}
			//Anexo la listaPaginas a un segmento que vamos a crear

			t_list *filter_list = list_filter(listaSegmentos, (void*) es_igual_pid); // filtro y me quedo con una lista de pid iguales
			int segmentos = list_size(filter_list);

			t_segmento *seg = crearSegmento(pid, (uint32_t) segmentos, listaPaginas);

			list_add(lista, seg);

			//Elimino la lista temporal
			list_clean(filter_list);


			iniciarBasesegmento();
			uint32_t direccion = armarDireccion(segmento, pagina, offset);
			retornarDireccion(pid, direccion);
			log_debug(logfile,"reservarSegmento()==>Se creo un segmento exitosamente...");
			}
			else{
				//error
				log_error(logfile,"crearSegmento()-No se puede crear el segmendo, debido a exceder el valor maximo de segmento, pedido por el proceso: %i ..",pid);

			}
	}
	else{
		//error
		log_error(logfile,"crearSegmento()-No se puede crear el segmendo, debido a exceder el valor maximo de memoria disponible, pedido por el proceso: %i ..",pid);

	}
}

bool es_igual_pid(t_segmento *p) {
		return (p->id == pid);
	}

bool es_igual_pid_and_seg(t_segmento *p) {
		return ((p->id == pid) && (p->num_segmento == num_segmento));
	}

bool es_igual_pag(t_pagina *p) {
		return (p->num_pagina == pagina);
	}

bool pagina_uno(t_segmento *p) { ///Mal pero sirve de ejemplo ->
		return (p->paginas->num_pagina == 1);
	}

bool pagina_dos(t_pagina *p) {
		return (p->num_pagina == 2);
	}

bool memoria_ocupada(t_memoria *p) {
		return (p->bit == 1 );
	}

bool memoria_vacia(t_memoria *p) {
		return (p->bit == 0 );
	}

t_segmento *crearSegmento(uint32_t id, uint32_t segmento, t_lista *pagina)
{
	t_segmento *new = malloc( sizeof(t_segmento) );
	new->id= id;
	new->num_segmento = segmento;
	new->paginas = pagina;

	return new;
}

t_pagina *crearPagina(uint32_t pagina)
{ //0= reservado 1= memoria 2= swap
	t_pagina *new = malloc( sizeof(t_pagina) );
	new->num_pagina = pagina;
	new->bit = 0;
	new->bonus = 0;
	new->num_marco = 0;

	return new;
}

t_memoria *crearMarco(uint32_t marco, uint32_t){

	t_memoria *new = malloc( sizeof(t_memoria) );
	new->num_marco = marco;
	new->bit = 0; //0 o 1
	return new;
}


// Te devuelve un num_byte de un value...
// Byte1 = segmento (12) Byte2 = pagina (12) Byte3 = offset (8)
uint32_t dameByte(uint32_t value, int num_byte) {

	uint32_t byte1;
	uint32_t byte2;
	uint32_t byte3;

	switch(num_byte){

		case 1:
				byte1 = (value >> 20);
				return byte1;

		case 2:
				byte2 = (value >> 8) & 0xfff;
				return byte2;

		case 3:
				byte3 = value & 0xff;
				return byte3;

	/* // Example value: 0x01020304
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
	}
}

uint32_t armarDireccion(uint32_t segmento, uint32_t pagina, uint32_t offset){
	//Movemos los bits necesarios
	uint32_t seg = (value << 20);
	uint32_t pag = (value << 8);

	//Procedemos a mezclar
	uint32_t aux = (seg | pag);
	uint32_t direccion = (aux | offset);

	return direccion;
}

void iniciarBasesegmento(){

	uint32_t segmento = 0;
	uint32_t pagina = 0;
	uint32_t offset = 0;
}

void retornarDireccion(uint32_t pid,uint32_t direccion_logica) {

	int stream_size = 2*REG_SIZE;
	char *stream = malloc(stream_size);
	memcpy(stream,&pid,REG_SIZE);
	memcpy(stream + REG_SIZE,&direccion_logica,REG_SIZE);

	t_msg *new_msg = crear_mensaje(id,stream,stream_size);

	enviar_mensaje(sockt,new_msg);
	destroy_message(new_msg);

}

void destruirSegmento(uint32_t direccion_logica){

	uint32_t num_segmento = dameByte(direccion_logica, 1)
	t_segmento *delete = list_remove_by_condition(listaSegmentos, (void*) es_igual_pid_and_seg);

				if (delete != NULL){

					free(delete);
					log_debug(logfile,"destruirSegmento()==>Se destruyo un segmento exitosamente...");

				}
				else{
					//error no se encontro segmento, segmentation fault??
					log_error(logfile,"destruirSegmento()-No se puede eliminar el segmendo pedido por el proceso: %i ..",pid);
				}

}

void escribirMemoria(uint32_t direccion_logica, char* bytes, uint32_t tamano){

	//Consigo el num de segmento
	uint32_t segmento = dameByte(direccion_logica, 1);
	//Con el pid y el num de segmento busco el nodo correspondiente
	t_segmento *seg = list_find(listaSegmentos, (void*) es_igual_pid_and_seg);
	//Consigo el num de pagina
	uint32_t pagina = dameByte(direccion_logica, 2);
	//Busco la pagina correspondiente
	t_pagina *pag = list_find(seg->paginas, (void*) es_igual_pag);

	if (pag != NULL){
		log_debug(logfile,"escribirMemoria()==>Se encontro la posicion logica en la memoria");
		}
		else{
		log_error(logfile,"escribirMemoria()==>No e encontro la posicion logica en la memoria");
		}

	//Te pido el offset---
	uint32_t offset = dameByte(direccion_logica, 3);

	if (pag->bit == 0){ //Se reservo pero no se accedio todavia

		t_memoria memoria = buscarMarcoDisponible();
		if (marco != NULL){
			//Calculo la direccion exacta para grabar (MARCO GATTI me obligo :( )
			uint32_t dezplazamiento = 256 * (memoria.num_marco); //Calculo el dezplazamiento "virtual" dentro de la memoria
			char *marco = point + dezplazamiento; // Calculo la direccion exacta a un marco, usando el point (del malloc al principio)
			grabarRAM(marco, offset, tamano, bytes);

			//Actualizo el registro de la estructura principal
			pag->num_marco = memoria.num_marco
			}
		else{
			//Tengo que manejar la swap, implementar algoritmo y desarrollar la busqueda y creacion de archivos swap


			}

			pag->bit = 1; // todo salio ok, se grabo el marco
			log_debug(logfile,"escribirMemoria(0)==>Se grabo en la memoria");

		}
	else{

		if (pag->bit == 1){

			uint32_t dezplazamiento = 256 * (pag->num_marco); //Calculo el dezplazamiento "virtual" dentro de la memoria
			char *marco = point + dezplazamiento; // Calculo la direccion exacta a un marco, usando el point (del malloc al principio)
			grabarRAM(marco, offset, tamano, bytes);
			log_debug(logfile,"escribirMemoria(1)==>Se grabo en la memoria"");
		}else{
			//Tengo que manejar la swap,

		}


	}





}

uint32_t memoriaDisponible(){

	list_size();

}

uint32_t calcularRAMocupada(){ //Utilizo la lista secundaria de la RAM || En terminos de numero de paginas

	t_list *ram_list = list_filter(listaMemoriaPrincipal, (void*) memoria_ocupada);
	uint32_t size = list_size(ram_list);
	list_clean(ram_list);
	return size;
}

uint32_t calcularSWAPocupada(){ //Utilizo la lista principal, utilizando el bit == 2 || En terminos de numero de paginas
	t_list lista_aux = listaSegmentos->head;
	t_list swap_total;

	while (lista_aux != NULL){
		// recorro cada segmento y filtro las paginas
		t_list *swap_list = list_filter(lista_aux->data->paginas, (void*) pagina_dos);
		// esa lista la agrego a swap_total
		list_add_all(swap_total, swap_list);
		lista_aux = lista_aux->next;

	}

	uint32_t size = list_size(swap_total);
	list_clean(swap_total);
	return size;
}

uint32_t espacioTotalOcupado(){ //|| En terminos de numero de paginas
	/*En este caso utilizo la estructura principal, solo cuento todas las paginas que tiene
	     ya que no puedo ceder mas paginas de mi limite (swap + ram)*/
	uint32_t total = 0;
	t_list lista_aux = listaSegmentos->head;
	while (lista_aux != NULL){

		uint32_t pag = list_size(lista_aux->data->paginas);
		total = total + pag;
		lista_aux = lista_aux->next;
		}
	return total;
}



void grabarRAM(char* point, uint32_t mem_pos, uint32_t size,  char* dato){

	char* pointer = point + mem_pos;

	memcpy(pointer, dato, size);
}

t_memoria buscarMarcoDisponible(){

	t_memoria *data = list_find(listaMemoriaPrincipal, (void*) memoria_vacia);
	return data;
}

void crearArchivoSWAP(uint32_t pid, uint32_t segmento, uint32_t pagina){
	// 6+4+4+3+1+4+|n  //Cantidad total de chars en aux

	strcat(adress,"touch ");	//touch, crea un nuevo archivo

	strcat(adress, armarSWAPath(pid, segmento, pagina));	// strcat, concatena swap a aux

	system(adress);
}

char *armarSWAPath(uint32_t pid, uint32_t segmento, uint32_t pagina){

		snprintf(cat,5,"%04d",pid);
		strcat(aux, cat);

		snprintf(cat,5,"%04d",segmento);
		strcat(aux, cat);

		snprintf(cat,5,"%03d",pagina);
		strcat(aux, cat);

		strcat(aux, swap);

		return aux;
}

