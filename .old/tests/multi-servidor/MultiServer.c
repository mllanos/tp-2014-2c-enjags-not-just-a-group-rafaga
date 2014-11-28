
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>

#define PUERTO "6667"
#define BACKLOG 5			// Define cuantas conexiones vamos a mantener pendientes al mismo tiempo
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar
#define MAX_CLIENTES 10

void nuevoCliente (int servidor, int *clientes, int *nClientes);
int dameMaximo (int *tabla, int n);
void compactaClaves (int *tabla, int *n);
int Escribe_Socket (int fd, char *Datos, int Longitud);

int main(){

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE;		// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(NULL, PUERTO, &hints, &serverInfo); // Notar que le pasamos NULL como IP, ya que le indicamos que use localhost en AI_PASSIVE


	int listenningSocket;
	listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo); // Ya no lo vamos a necesitar

	int error = listen(listenningSocket, BACKLOG);		// IMPORTANTE: listen() es una syscall BLOQUEANTE.
		if (error > 0) printf("listenin\n");

	/*  Utilizo magia negra y comienzo a usar SELECT*/

	int socketCliente[MAX_CLIENTES];/* Descriptores de sockets con clientes */
	int numeroClientes = 0;			/* Número clientes conectados */
	fd_set descriptoresLectura;	/* Descriptores de interes para select() */
	int maximo;							/* Número de descriptor más grande */
	int i = 0;

	/*  Controlo el ingreso o el suceso de algo que paso en nuestro socket, para PRUEBA de 2*/

		int p = 0;
		while (p<2){

	/* Cuando un cliente cierre la conexión, se pondrá un -1 en su descriptor
			 * de socket dentro del array socketCliente. La función compactaClaves()
			 * eliminará dichos -1 de la tabla, haciéndola más pequeña.
			 *
			 * Se eliminan todos los clientes que hayan cerrado la conexión */
			compactaClaves (socketCliente, &numeroClientes);

			/* Se inicializa descriptoresLectura */
			FD_ZERO (&descriptoresLectura);

			/* Se añade para select() el socket servidor */
			FD_SET (listenningSocket, &descriptoresLectura);

			/* Se añaden para select() los sockets con los clientes ya conectados */
			for (i=0; i<numeroClientes; i++)
				FD_SET (socketCliente[i], &descriptoresLectura);

			/* Se el valor del descriptor más grande. Si no hay ningún cliente,
			 * devolverá 0 */

			maximo = dameMaximo (socketCliente, numeroClientes);

			if (maximo < listenningSocket) maximo = listenningSocket;


			select (numeroClientes+1, &descriptoresLectura, NULL, NULL, NULL);

	/* Se tratan los clientes */

	/* Se comprueba si algún cliente ya conectado ha enviado algo */
	for (i=0; i<numeroClientes; i++)
		{
		if (FD_ISSET (socketCliente[i], &descriptoresLectura))
			{
			char package[PACKAGESIZE];
			int status = 1;
			/* Se lee lo enviado por el cliente y se escribe en pantalla */
			if (recv(socketCliente[i], (void*) package, PACKAGESIZE, 0) > 0)
				{ // Estructura que manjea el status de los recieve.
				printf("Cliente conectado. Esperando mensajes:\n");
				printf("%s", package);
				while (status != 0){
					status = recv(socketCliente[i], (void*) package, PACKAGESIZE, 0);
					if (status != 0) printf("%s", package);
									}
				}
			else
				{
				/* Se indica que el cliente ha cerrado la conexión y se
				 * marca con -1 el descriptor para que compactaClaves() lo
				 * elimine */
				printf ("Cliente %d ha cerrado la conexión\n", i+1);
				socketCliente[i] = -1;
				}
			}
		}

	/* Se comprueba si algún cliente nuevo desea conectarse y se lo admite */
		if (FD_ISSET (listenningSocket, &descriptoresLectura)) nuevoCliente (listenningSocket, socketCliente, &numeroClientes);

		p++;
		}


	close(listenningSocket);

	/* See ya! */

	return 0;
}


/*
 * Crea un nuevo socket cliente.
 * Se le pasa el socket servidor y el array de clientes, con el número de
 * clientes ya conectados.
 */
void nuevoCliente (int servidor, int *clientes, int *nClientes)
{
	/* Acepta la conexión con el cliente, guardándola en el array */

	struct sockaddr_in addr;			// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);

	clientes[*nClientes] = accept(servidor, (struct sockaddr *) &addr, &addrlen);
	(*nClientes)++;

	/* Si se ha superado el maximo de clientes, se cierra la conexión*/

	if ((*nClientes) >= MAX_CLIENTES)
	{
		close (clientes[(*nClientes) -1]);
		(*nClientes)--;
		return;
	}

	/* Envía su número de cliente al cliente */
	Escribe_Socket (clientes[(*nClientes)-1], (char *)nClientes, sizeof(int));

	/* Escribe en pantalla que ha aceptado al cliente y vuelve */
	printf ("Aceptado cliente %d\n", *nClientes);
	return;
}

/*
 * Función que devuelve el valor máximo en la tabla.
 * Supone que los valores válidos de la tabla son positivos y mayores que 0.
 * Devuelve 0 si n es 0 o la tabla es NULL */
int dameMaximo (int *tabla, int n)
{
	int i;
	int max;

	if ((tabla == NULL) || (n<1))
		return 0;

	max = tabla[0];
	for (i=0; i<n; i++)
		if (tabla[i] > max)
			max = tabla[i];

	return max;
}

/*
 * Busca en array todas las posiciones con -1 y las elimina, copiando encima
 * las posiciones siguientes.
 * Ejemplo, si la entrada es (3, -1, 2, -1, 4) con *n=5
 * a la salida tendremos (3, 2, 4) con *n=3
 */
void compactaClaves (int *tabla, int *n)
{
	int i,j;

	if ((tabla == NULL) || ((*n) == 0))
		return;

	j=0;
	for (i=0; i<(*n); i++)
	{
		if (tabla[i] != -1)
		{
			tabla[j] = tabla[i];
			j++;
		}
	}

	*n = j;
}

int Escribe_Socket (int fd, char *Datos, int Longitud)
{
	int Escrito = 0;
	int Aux = 0;

	/*
	* Comprobacion de los parametros de entrada
	*/
	if ((fd == -1) || (Datos == NULL) || (Longitud < 1))
		return -1;

	/*
	* Bucle hasta que hayamos escrito todos los caracteres que nos han
	* indicado.
	*/
	while (Escrito < Longitud)
	{
		Aux = write (fd, Datos + Escrito, Longitud - Escrito);
		if (Aux > 0)
		{
			/*
			* Si hemos conseguido escribir caracteres, se actualiza la
			* variable Escrito
			*/
			Escrito = Escrito + Aux;
		}
		else
		{
			/*
			* Si se ha cerrado el socket, devolvemos el numero de caracteres
			* leidos.
			* Si ha habido error, devolvemos -1
			*/
			if (Aux == 0)
				return Escrito;
			else
				return -1;
		}
	}

	/*
	* Devolvemos el total de caracteres leidos
	*/
	return Escrito;
}
