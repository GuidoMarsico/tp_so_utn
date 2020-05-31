#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#define PUERTO "6667"
#define BACKLOG 5
#define PACKAGESIZE 1024


typedef struct{
	int32_t pid;
	u_int32_t tam_o_base;
}__attribute__ ((__packed__)) t_reservar_destruir;

typedef struct{
	int32_t pid;
	u_int32_t dirLogica;
	int tam;
}__attribute__ ((__packed__)) t_leer;

typedef struct{
	int32_t pid;
	u_int32_t dirLogica;
	int tam;
	char* bytesAEscribir;
}__attribute__ ((__packed__)) t_escribir;

typedef struct{
	int longitud;
	char* dataARecibir;
}t_stream;

int socketCliente;
t_reservar_destruir* reservar_destruir_DESserializacion(t_stream* stream){
	t_reservar_destruir* rd = malloc ( sizeof (t_reservar_destruir) );
	int offset = 0, tmp_size = 0;

	offset = tmp_size;
	memcpy( &rd->pid , stream->dataARecibir + offset , tmp_size = sizeof (int32_t) );

	offset += tmp_size;
	memcpy (&rd->tam_o_base , stream->dataARecibir + offset , tmp_size = sizeof (u_int32_t) );


	return rd;
}

t_leer* leer_DESserializacion(t_stream* stream){
	t_leer* l = malloc ( sizeof (t_leer) );
	int offset = 0, tmp_size = 0;

	offset = tmp_size;
	memcpy( &l->pid , stream->dataARecibir + offset , tmp_size = sizeof (int32_t) );

	offset += tmp_size;
	memcpy (&l->dirLogica , stream->dataARecibir + offset , tmp_size = sizeof (u_int32_t) );

	offset += tmp_size;
	memcpy (&l->tam , stream->dataARecibir + offset , tmp_size = sizeof (int) );


	return l;
}

t_escribir* escribir_DESserializacion(t_stream* stream){
	t_escribir* e = malloc ( sizeof (t_escribir) );
	int offset = 0, tmp_size = 0;

	offset = tmp_size;
	memcpy( &e->pid , stream->dataARecibir + offset , tmp_size = sizeof (int32_t) );

	offset += tmp_size;
	memcpy (&e->dirLogica , stream->dataARecibir + offset , tmp_size = sizeof (u_int32_t) );

	offset += tmp_size;
	memcpy (&e->tam , stream->dataARecibir + offset , tmp_size = sizeof (int) );

	offset += tmp_size;
	for (tmp_size = 1 ; (stream->dataARecibir + offset)[tmp_size-1] != '\0';tmp_size++);
	e->bytesAEscribir = malloc (tmp_size);
	memcpy(e->bytesAEscribir,stream->dataARecibir + offset,tmp_size);
	return e;
}

int main(){
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(NULL, PUERTO, &hints, &serverInfo);
	int listenningSocket;
	listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);
	listen(listenningSocket, BACKLOG);
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	socketCliente = accept(listenningSocket, (struct sockaddr *) &addr, &addrlen);

	int opcion;
	recv(socketCliente,&opcion, sizeof(int), MSG_WAITALL);

	printf("\nLa opción recibida fue: %d",opcion);

	if (opcion == 1){// OPCION RESERVAR (1)
		printf("\nSe reserva...");
		t_stream* stream = malloc (sizeof (stream));
		t_reservar_destruir* rd = malloc (sizeof (t_reservar_destruir));
		char rdRecibido[sizeof(t_reservar_destruir)];
		fflush(stdout);
		recv(socketCliente, (void*)&rdRecibido, sizeof(t_reservar_destruir), MSG_WAITALL);
		stream->dataARecibir = rdRecibido;
		rd = reservar_destruir_DESserializacion(stream);
		printf("\nPID: %d",rd->pid);
		printf("\nTamAreservar (3 dig): %d\n",rd->tam_o_base);
		free(stream);
		free(rd);

	}else if (opcion == 2){//OPCION DESTRUIR (2)
		printf("\nSe destruye...");
		t_stream* stream = malloc (sizeof (stream));
		t_reservar_destruir* rd = malloc (sizeof (t_reservar_destruir));
		char rdRecibido[sizeof(t_reservar_destruir)];
		fflush(stdout);
		recv(socketCliente, (void*)&rdRecibido, sizeof(t_reservar_destruir), MSG_WAITALL);
		stream->dataARecibir = rdRecibido;
		rd = reservar_destruir_DESserializacion(stream);
		printf("\nPID: %d",rd->pid);
		printf("\nBASE (6 dig min): %d\n",rd->tam_o_base);
		free(stream);
		free(rd);

	}else if (opcion == 3){//OPCION LEER (3)
		printf("\nSe lee...");
		t_stream* stream = malloc (sizeof (stream));
		t_leer* l = malloc (sizeof (t_leer));
		char lRecibido[sizeof(t_leer)];
		fflush(stdout);
		recv(socketCliente, (void*)&lRecibido, sizeof(t_leer), MSG_WAITALL);
		stream->dataARecibir = lRecibido;
		l = leer_DESserializacion(stream);
		printf("\nPID: %d",l->pid);
		printf("\nDir lógica: (6 dig min): %d",l->dirLogica);
		printf("\nTam a leer: %d\n",l->tam);
		free(stream);
		free(l);

	}else if (opcion == 4){//OPCION ESCRIBIR (4)
		printf("\nSe escribe...");
		t_stream* stream = malloc (sizeof (stream));
		t_escribir* e = malloc (sizeof (t_escribir));
		int tamArecibir = 0;
		recv(socketCliente, &tamArecibir, sizeof(int), MSG_WAITALL);
		char eRecibido[tamArecibir];
		printf("\nSe debe recibir: %d",tamArecibir);
		int ok = 1;
		send(socketCliente,&ok,sizeof(int),0);
		int recibido = recv(socketCliente, (void*)&eRecibido, tamArecibir, MSG_WAITALL);
		printf("\nTam stream: %d\tRecibido: %d",sizeof(stream),recibido);
		stream->dataARecibir = eRecibido;
		e = escribir_DESserializacion(stream);
		printf("\nPID: %d",e->pid);
		printf("\nDir lógica: (6 dig min): %d",e->dirLogica);
		printf("\nTam a leer: %d",e->tam);
		printf("\nContenido a Escribir: %s\n",e->bytesAEscribir);

		free(stream);
		free(e);

	}


	close(socketCliente);
	close(listenningSocket);
	return 0;
}
