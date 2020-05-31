#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define IP "127.0.0.1"
#define PUERTO "6667"
#define PACKAGESIZE 1024


typedef struct{
	int32_t pid;
	u_int32_t tam_o_base;
}__attribute__ ((__packed__)) t_reservar_destruir; //esa anotación adelante es para que no asigne bytes de más

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
	char* dataAEnviar;
}t_stream;

int serverSocket;

t_escribir* escribir_DESserializacion(t_stream* stream){
	t_escribir* e = malloc ( sizeof (t_escribir) );
	int offset = 0, tmp_size = 0;

	offset = tmp_size;
	memcpy( &e->pid , stream->dataAEnviar + offset , tmp_size = sizeof (int32_t) );

	offset += tmp_size;
	memcpy (&e->dirLogica , stream->dataAEnviar + offset , tmp_size = sizeof (u_int32_t) );

	offset += tmp_size;
	memcpy (&e->tam , stream->dataAEnviar + offset , tmp_size = sizeof (int) );

	offset += tmp_size;
	for (tmp_size = 1 ; (stream->dataAEnviar + offset)[tmp_size-1] != '\0';tmp_size++);
	e->bytesAEscribir = malloc (tmp_size);
	memcpy(e->bytesAEscribir,stream->dataAEnviar + offset,tmp_size);
	return e;
}



t_stream* escribir_serializacion(t_escribir* e,t_stream* stream){
	char* data = malloc ( sizeof ( int32_t ) + sizeof ( u_int32_t ) + sizeof( int ) + strlen(e->bytesAEscribir)+1);
	int offset = 0, tmp_size = 0;

	offset = tmp_size;
	memcpy (data + offset, &e->pid, tmp_size = sizeof ( int32_t ));

	offset += tmp_size;
	memcpy (data+offset, &e->dirLogica, tmp_size = sizeof (u_int32_t));

	offset += tmp_size;
	memcpy (data+offset, &e->tam, tmp_size = sizeof (int));

	offset += tmp_size;
	memcpy (data+offset, e->bytesAEscribir, tmp_size = strlen(e->bytesAEscribir)+1);

	stream->longitud = offset + tmp_size;
	stream->dataAEnviar = data;

	return stream;
}

void escribir_enviar(int32_t pid, u_int32_t dirLogica, int tam, char* bytesAEscribir){
	t_escribir* e = malloc (sizeof(t_escribir));
	int ok = 0;
	e->pid = pid;
	e->dirLogica = dirLogica;
	e->tam = tam;
	e->bytesAEscribir = bytesAEscribir;
	printf("\nBytesAescribir: %s\n",e->bytesAEscribir);
	t_stream* stream = malloc ( sizeof ( t_stream ) );
	stream = escribir_serializacion(e,stream);
	printf("\nSe debe enviar: %d",stream->longitud);

	send(serverSocket, &stream->longitud, sizeof(int), 0);
	recv(serverSocket,&ok,sizeof(int),MSG_WAITALL);

	int enviado = send(serverSocket, stream->dataAEnviar, stream->longitud, 0);
	printf("\nLo que se envió tiene un tamaño de %d",enviado);
	free(e);
	t_escribir* e2 = malloc (sizeof (t_escribir));
	e2 = escribir_DESserializacion(stream);
	printf("\nBytesAescribir: %s\n",e2->bytesAEscribir);
	free(e2);
	free(stream);
}


t_stream* reservar_destruir_serializacion(t_reservar_destruir* rd,t_stream* stream){
	char* data = malloc ( sizeof ( int32_t ) + sizeof ( u_int32_t ) );
	int offset = 0, tmp_size = 0;

	offset = tmp_size;
	memcpy (data + offset, &rd->pid, tmp_size = sizeof ( int32_t ));

	offset += tmp_size;
	memcpy (data+offset, &rd->tam_o_base, tmp_size = sizeof (u_int32_t));

	stream->longitud = offset + tmp_size;
	stream->dataAEnviar = data;

	return stream;
}

void reservar_destruir_enviar (int32_t pid, u_int32_t tamAreservar_o_base){

	t_reservar_destruir* rd = malloc (sizeof(t_reservar_destruir));
	rd->pid = pid;
	rd->tam_o_base = tamAreservar_o_base;
	t_stream* stream = malloc ( sizeof ( t_stream ) );
	stream = reservar_destruir_serializacion(rd,stream);
	send(serverSocket, stream->dataAEnviar, stream->longitud, 0);

	free(rd);
	free(stream);

}

t_stream* leer_serializacion(t_leer* l,t_stream* stream){
	char* data = malloc ( sizeof ( int32_t ) + sizeof ( u_int32_t ) + sizeof (int) );
	int offset = 0, tmp_size = 0;

	offset = tmp_size;
	memcpy (data + offset, &l->pid, tmp_size = sizeof ( int32_t ));

	offset += tmp_size;
	memcpy (data+offset, &l->dirLogica, tmp_size = sizeof (u_int32_t));

	offset += tmp_size;
	memcpy (data+offset, &l->tam, tmp_size = sizeof (int));

	stream->longitud = offset + tmp_size;
	stream->dataAEnviar = data;

	return stream;
}

void leer_enviar(int32_t pid, u_int32_t dirLogica, int tam){
	t_leer* l = malloc (sizeof(t_leer));
	l->pid = pid;
	l->dirLogica = dirLogica;
	l->tam = tam;
	t_stream* stream = malloc ( sizeof ( t_stream ) );
	stream = leer_serializacion(l,stream);
	send(serverSocket, stream->dataAEnviar, stream->longitud, 0);

	free(l);
	free(stream);
}


int main(void) {

	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(IP, PUERTO, &hints, &serverInfo);
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);

	int opcion;

//		//ENVIO PARA RESERVAR O DESTRUIR
//		opcion = 2; //reservar 1 destruir 2
//		send(serverSocket, &opcion, sizeof(int), 0);
//		reservar_destruir_enviar(0,100000);//syscalls,base

//		//ENVIO PARA LEER
//		opcion = 3;
//		send(serverSocket, &opcion, sizeof(int), 0);
//		leer_enviar(0,100100,50);

	//ENVIO PARA ESCRIBIR
	opcion = 4;
	send(serverSocket, &opcion, sizeof(int), 0);
	char* cadena = "escribime esta ahsdjasdjasdhjasdasdasdsad4";
	escribir_enviar(1,100100,strlen(cadena),cadena); //HAY QUE VER SI LE PONEMOS +1 en EL STRLEN


	close(serverSocket);
	return EXIT_SUCCESS;
}
