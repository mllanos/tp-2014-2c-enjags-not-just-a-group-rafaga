
#include "umv.h"
#include <stdlib.h>

#define STDIN            0
#define SEG_MAX			 1048576
#define PAG_MAX          256


int  socketHilo,socketKernel; //los que retorna el accept dentro de AtenderConexion
char *point;
char ip[16], algoritmo[8];
int puerto, cmemoria, cswap;
t_log  *logfile;
t_list *listaSegmentos; //O lista de segmentos
fd_set  descriptoresLectura;
u_int16_t pid;
int mem_total = 0;


typedef struct {		//Esto representa a un segmento (una fila de la tabla)
	u_int16_t id;
	u_int16_t num_segmento;
	t_lista *paginas;
} t_segmento;

typedef struct {		//Esto representa a una pagina
	u_int16_t num_pagina;
	u_int16_t bit;
	u_int16_t bonus;
	u_int32_t direccion;
} t_pagina;

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
	mem_total = cmemoria + cswap; //actualizo la memoria total
	//printf("archivo de configuracion levantado: ip:%s ...);
	log_debug(logfile,"cargarConficuracion(char* path)-Proceso exitoso, con ip:%s puerto:%i tamanio de memoria:%i swap:%i algoritmo:%i ...",ip,puerto,cmemoria,cswap,algoritmo);
}

void crearEstructuras(){

	listaSegmentos=list_create();
	point = (char*) malloc(cmemoria);  // Creo memoria principal (malloc).

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

void reservarMemoria(u_int16_t id, int tamaño) {
	//acordar usar lista de segmentos para calcular el espacio libre de memoria, vercuales estan en (0)y (1) para saber le de la swap, porque tienen un limite.
	//usar esto al momento de pasar las paginas a memoria y tambien luego al reemplazarlas. para eso me sirve el -1, osea saber en la lista que paginas todavia no se escribieron.
	//el algoritmo de reemplazo influye al reemplazar no ahora, asi que el bonus lo inicio en 0.
	int pag = 0;
	int c_paginas

	if tamaño < SEG_MAX{

		div_t c = div(tamaño, PAG_MAX); //Calculo cuantas paginas voy a usar y se lo asigno a c_paginas

		if (c.rem == 0) {c_paginas = c.quot;} else {c_paginas = c.quot + 1;} //Si la ultima pagina va a estar media vacia. Fragmentacion externa?

		t_list listaPaginas = list_create();

		//Creo las paginas correspondientes
		for(int i=1; i <= c_paginas; i++)
			{
				t_segmento *pagina = crearPagina(i);
				list_add(listaPaginas, pagina);
			}
		//Anexo la listaPaginas a un segmento que vamos a crear

		t_list *filter_list = list_filter(listaSegmentos, (void*) es_igual); // filtro y me quedo con una lista de pid iguales
		int segmentos = list_size(filter_list);

		t_segmento *seg = crearSegmento(id, (u_int16_t) segmentos, listaPaginas);

		list_add(lista, seg);

		//Elimino la lista temporal
		list_clean(filter_list);

	}
	else{
		//error
		log_error(logfile,"crearSegmento()-No se puede crear el segmendo pedido por el proceso: %i ..",pid);
		listaSegmentos
	}
}

bool es_igual(t_segmento *p) {
		return (p->id == pid);
	}

t_segmento *crearSegmento(u_int16_t id, u_int16_t segmento, t_lista *pagina)
{
	t_segmento *new = malloc( sizeof(t_segmento) );
	new->id= id;
	new->num_segmento = segmento;
	new->paginas = pagina;

	return new;
}

t_segmento *crearPagina(u_int16_t pagina)
{
	t_segmento *new = malloc( sizeof(t_pagina) );
	new->num_pagina = pagina;
	new->bit = -1;
	new->bonus = 0;
	new->direccion = 0;

	return new;
}
