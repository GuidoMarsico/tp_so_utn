#include "globalesCPU.h"

t_escribir* escribir_DESserializacion(t_stream2* stream){
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

t_stream2* escribir_serializacion(t_escribir* e,t_stream2* stream, int tamanioCodigo){
	char* data = malloc ( sizeof ( int32_t ) + sizeof ( u_int32_t ) + sizeof( int ) + tamanioCodigo);
	int offset = 0, tmp_size = 0;
	offset = tmp_size;
	memcpy (data + offset, &e->pid, tmp_size = sizeof ( int32_t ));
	offset += tmp_size;
	memcpy (data+offset, &e->dirLogica, tmp_size = sizeof (u_int32_t));
	offset += tmp_size;
	memcpy (data+offset, &e->tam, tmp_size = sizeof (int));
	offset += tmp_size;
	memcpy (data+offset, e->bytesAEscribir, tmp_size = tamanioCodigo);
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
	t_stream2* stream = malloc (sizeof(t_stream2));
	stream = escribir_serializacion(e,stream,tam+1);
	printf("\nSe debe enviar: %d",stream->longitud);
	send(socketMSP, &stream->longitud, sizeof(int), 0);
	recv(socketMSP,&ok,sizeof(int),MSG_WAITALL);
	int enviado = send(socketMSP, stream->dataAEnviar, stream->longitud, 0);
	printf("\nLo que se envió tiene un tamaño de %d",enviado);
	free(e);
	t_escribir* e2 = malloc (sizeof (t_escribir));
	e2 = escribir_DESserializacion(stream);
	printf("\nBytesAescribir: %s\n",e2->bytesAEscribir);
	free(e2);
	free(stream);
}

t_stream2* reservar_destruir_serializacion(t_reservar_destruir* rd, t_stream2* stream){
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
	t_stream2* stream = malloc ( sizeof ( t_stream2 ) );
	stream = reservar_destruir_serializacion(rd,stream);
	send(socketMSP, stream->dataAEnviar, stream->longitud, 0);
	free(rd);
	free(stream);
}

t_stream2* leer_serializacion(t_leer* l, t_stream2* stream){
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
	t_stream2* stream = malloc ( sizeof ( t_stream2 ) );
	stream = leer_serializacion(l,stream);
	send(socketMSP, stream->dataAEnviar, stream->longitud, 0);
	free(l);
	free(stream);
}

void escribirEnMemoria(char* contenido, int32_t pid, u_int32_t dir, int tamCod){
	int opcion = 4;
	send(socketMSP, &opcion, sizeof(int), 0);
	escribir_enviar(pid,dir,tamCod,contenido);
	int respuesta = 0;
	recv(socketMSP,&respuesta,sizeof(int),MSG_WAITALL);
	if (respuesta == -1){
		//TODO: HACER MANEJO
		printf("\nSEGMENTATION FAULT:");
		printf("\nLa MSP no logró escribir en memoria.\n");
	}
}

char* leerDeMemoria(int32_t pid, u_int32_t dirLogica, int tamanio){
	int opcion = 3;
	send(socketMSP, &opcion, sizeof(int), 0);
	leer_enviar(pid,dirLogica,tamanio);
	int rta = 0;
	char* contenido = malloc(tamanio+1);
	recv(socketMSP,&rta,sizeof(int),MSG_WAITALL);
	if(rta == -1){
		printf("\nNo se pudo leer de memoria");
		contenido = NULL;
	}else{
		send(socketMSP,&rta, sizeof(int),0);
		recv(socketMSP,contenido,tamanio+1,MSG_WAITALL); //TODO: FIJARSE QUE LLEGUE BIEN
		printf("\nCONTENIDO RECIBIDO: %s\n",contenido);
//		memcpy(contenido, contenidoRecibido, tamanio);
	}
	return contenido;
}

u_int32_t reservarDireccion(int tamanio, int32_t pid){
	u_int32_t dir;
	int opcion = 1;
	send(socketMSP, &opcion, sizeof(int), 0);
	reservar_destruir_enviar(pid,tamanio);//syscalls,base
	recibir(socketMSP,sizeof(u_int32_t),(void*)&dir);
	printf("DIRECCION: %x\n",dir);
	return dir;
}

int eliminarDeMemoria(int32_t pid, u_int32_t base){
	int opcion = 2;
	send(socketMSP, &opcion, sizeof(int), 0);
	reservar_destruir_enviar(pid,base);//syscalls,base
	int rta = 0;
	recv(socketMSP,&rta,sizeof(int),MSG_WAITALL);
	if(rta == -1){
		printf("\nNo se pudo eliminar memoria\n");
		return -1;
	}
	return 0;
}
