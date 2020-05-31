#include "globalKernel.h"

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
	t_stream2* stream = malloc (sizeof(t_stream2));
	stream = escribir_serializacion(e,stream,tam+1);
	send(socketMSP, &stream->longitud, sizeof(int), 0);
	recv(socketMSP,&ok,sizeof(int),MSG_WAITALL);
	send(socketMSP, stream->dataAEnviar, stream->longitud, 0);
	free(e);
	t_escribir* e2 = malloc (sizeof (t_escribir));
	e2 = escribir_DESserializacion(stream);
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
