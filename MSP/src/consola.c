#include "msp.h"

void consolaMSP () {
	char command[35];
	char action[15];
	char stream[10];
	char *texto;
	u_int32_t pid;
	u_int32_t size;
	u_int32_t direccion;
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
				direccion=atoi(stream);
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
				direccion=atoi(stream);
				j++;
				i=0;
				stream[0]="\0";
			while(command[j]!="\0") {
				stream[i]=command[j];
				i++;
				j++;
			}
				size=atoi(stream);
				leerMemoria(pid,direccion,size);
		}


		else if (strcmp(action,"segmentsTable",i)==0) {
			t_link_element *element = listaSegmentos->head;
			printf("PID          N° Segmento              Tamano            Direccion Base");
			while (element != NULL) {
				printf(element->data->pid);
				printf(element->data->num_segmento);
				printf(sizeof(element->data));
				direccion = armarDireccion(element->data->num_segmento, element->data->paginas->num_pagina, 0);
				printf(direccion);
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
				t_list *filter_list = list_filter(listaSegmentos, es_igual_pid(listaSegmentos->head->data));
				t_link_element *element = filter_list->head;
				while (element != NULL) {
					printf(element->data->paginas->data->num_pagina);
					if (element->data->paginas->data->bit == 1){
						printf("Esta en memoria");
					} else if (element->data->paginas->data->bit == 2) {
						printf("Esta swappeada");
					}
					printf(element->data->num_segmento);
					element = element->next;
				}
		}


		else if (strcmp(action,"listFrames",i)==0) {
			t_link_element *element = listaMemoriaPrincipal->head;
			while (element != NULL) {
				printf(element->data->num_marco);
				if (element->data->bit == 0) {
					printf("El marco esta libre");
				} else if (element->data->bit == 1) {
					printf("El marco esta ocupado");
				}
				t_list *filter_list = list_filter(listaSegmentos, listaSegmentos->head->data->paginas->num_marco==listaMemoriaPrincipal->head->data->num_marco);
				printf(filter_list->head->data->id);
				element = element->next;
			}
		}

		else {
			printf("Error de comando");
			log_trace(logfile,"Error de comando");
		}

	}
}

