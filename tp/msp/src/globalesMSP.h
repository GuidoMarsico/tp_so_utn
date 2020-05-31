#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#define KGRN  "\x1B[32m"
#define KNRM  "\x1B[0m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define KRED  "\x1B[31m"
#define refmspConfig "/home/utnso/git/tp-2014-2c-rafagadeterror/tp/msp/msp.conf"

char* algoritmo;
int32_t memMaxima,memReservada;

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

t_escribir* escribir_DESserializacion(t_stream* stream, int tamanioCodigo){
	t_escribir* e = malloc ( sizeof (t_escribir) );
	int tamCod = tamanioCodigo;
	int offset = 0, tmp_size = 0;
	offset = tmp_size;
	memcpy( &e->pid , stream->dataARecibir + offset , tmp_size = sizeof (int32_t) );
	offset += tmp_size;
	tamCod -= tmp_size;
	memcpy (&e->dirLogica , stream->dataARecibir + offset , tmp_size = sizeof (u_int32_t) );
	offset += tmp_size;
	tamCod -= tmp_size;
	memcpy (&e->tam , stream->dataARecibir + offset , tmp_size = sizeof (int) );
	offset += tmp_size;
	tamCod -= tmp_size;
	for (tmp_size = 1 ; tmp_size < tamCod;tmp_size++);
	e->bytesAEscribir = malloc (tmp_size);
	memcpy(e->bytesAEscribir,stream->dataARecibir + offset,tmp_size);
	return e;
}

typedef struct{
	char* puertoEscucha;
	int32_t cantidadMemoria;
	int32_t cantidadSwapping;
	char* algoritmo;
	int32_t tamanioPagina;
}thread_parm_t;

typedef struct{
	int32_t numero;
	int32_t marco; //-2=no existe, -1=swap, 0..n= numero de marco
}pagina_t;

typedef struct{
	int32_t numero;
	int32_t pid;
	int32_t cantidadDePaginas;
	t_list *tablaDePaginas;
}segmentos_t;

typedef struct{
	int segmento;
	int pagina;
	int offset;
}direccion_t;

typedef struct{
	int32_t numero;
	int32_t pagina;
	int32_t segmento;
	int32_t bitDeUso;
	int32_t contadorLRU;
	int32_t pid;
}frame_t;

typedef struct{
	int32_t pid;
	int numSegm;
	int numPag;
}t_pagSwap;

int tamanioDePagina;
int contadorSegmentos=0;
int buscadorSwap, paginaBuscadaSwap, segmSwap, pidSwap;
int cantidadDeMarcos;
int kernelSocket=-1;
int pidDeSegmentoBuscado,contadorDeSegmentos,numeroSegmentoBuscado,pidDeSegmento,segmentoActual,numPagBuscada;
int32_t marcoBuscado;
int32_t pidADestruir,pidBuscado;
int32_t cantidadMaximaDePaginas;
int32_t cantMaxDePaginasSwapeadas;
int32_t cantidadArchivosSwapeados;

t_list *listaDeMarcos;
t_list *listaDePaginasSwap;
t_list *listaDeSegmentos;

char* memoriaPrincipal;

t_log* logs;
int malEstado = 0;

int32_t devolvemeElValorEnteroDe(char* clave,t_config* config){
	if( config_has_property(config,clave) ){
		int32_t valor = config_get_int_value(config,clave);
		return valor;
	} else {
		printf("No existe la propiedad %s",clave);
		log_error(logs,"No existe una propiedad");
		log_destroy(logs);
		exit(1);
	}
}

void imprimirCodigoCompleto(int tam,char* cod){
	int x;
	for(x=0; x < tam; x++){
		//Si es un \0, remplazalo por un '_'
		printf("%c", (cod[x] == '\0') ? '+' : cod[x]);}
	printf("\n");
}

void mostrarMemoria(){
	printf("\nCONTENIDO MEMORIA: \n");
	int sumadorOffset = 0;
	int tamTotal = memReservada;
	printf("Memoria Reservada: %d\n",memReservada);
	while (tamTotal > 0){
		char* marco = malloc(tamanioDePagina);
		memcpy(marco,memoriaPrincipal+sumadorOffset,tamanioDePagina);
		printf("\nContenido en un marco:\n");
		imprimirCodigoCompleto(tamanioDePagina,marco);
		tamTotal -= tamanioDePagina;
		sumadorOffset += tamanioDePagina;
		free(marco);
	}
}

int esMarcoBuscado(frame_t* elem){
	return elem->numero = marcoBuscado;
}

void _actualizarPaginaSwapp(pagina_t* elem){
	if(elem->numero==paginaBuscadaSwap){
		elem->marco=-1;
	}
}

segmentos_t *buscarNodo(int32_t pid){
	segmentos_t *nodo;
	int i=0;
	int encontro=0;
	while(!encontro && i<list_size(listaDeSegmentos)){
		nodo=list_get(listaDeSegmentos,i);
		if(nodo->pid==pid){
			encontro=1;
		}
		i++;
	}
	if(!encontro){
		log_error(logs,"no se encontro ningun segmento del pid: %d\n",pid);
	}
	return nodo;
}

void _impresionPaginas(pagina_t* elem){
	printf("Numero de página: %d\n",elem->numero);
	if(elem->marco == -2){
		printf("La página no fue cargada en memoria todavia\n");
	}else if(elem->marco == -1){
		printf("La página se encuentra swappeada\n");
	}else{
		printf("Marco en el que se encuentra la página: %d\n",elem->marco);
	}
}

int esPidBuscado(segmentos_t* elem){
	return elem->pid == pidBuscado;
}

int _esPaginaBuscada(pagina_t* elem){
	return elem->numero = numPagBuscada;
}

int _esSegmentoBuscado(segmentos_t* elem){
	return (elem->numero == numeroSegmentoBuscado && elem->pid == pidDeSegmento);
}

int _esSegmentoAImprimir(segmentos_t* elem){
	return (elem->numero == segmentoActual && elem->pid == pidDeSegmento);
}

void imprimirTablaPaginas(int32_t pid, int numero){
	numeroSegmentoBuscado = numero;
	pidDeSegmento = pid;
	segmentos_t* seg = list_find(listaDeSegmentos, (void*) _esSegmentoBuscado);
	list_iterate(seg->tablaDePaginas,(void*) _impresionPaginas);
}

void impresionSegmento(segmentos_t* elem){
	if(elem->pid == pidDeSegmento){
		printf("Numero de Segmento: %d\n", elem->numero);
		printf("Cantidad de Paginas: %d\n", elem->cantidadDePaginas);
		imprimirTablaPaginas(elem->pid,elem->numero);
	}
}

void imprimirTablaSegmentos(int32_t pid){
	pidDeSegmento = pid;
	printf("PID de los Segmentos: %d\n", pid);
	list_iterate(listaDeSegmentos, (void*) impresionSegmento);
}

void impresionMarco(frame_t* elem){
	printf("Numero de Marco: %d\n", elem->numero);
	if(elem->segmento == -1){
		printf("No hay ningun segmento ni pagina alojados en el marco\n");
	}else{
		printf("PID del Segmento Alojado: %d\n",elem->pid);
		printf("Numero de Segmento Alojado: %d\n", elem->segmento);
		printf("Numero de Pagina Alojada: %d\n", elem->pagina);
	}
	printf("Bit de Uso del Marco: %d\n", elem->bitDeUso);
	printf("Instantes que no se utiliza el marco: %d\n", elem->contadorLRU);
}

void mostrarEstadoDeMarcos(){
	list_iterate(listaDeMarcos, (void*) impresionMarco);
}

t_config* config;
thread_parm_t *parm;

void _marcoDestroyer(frame_t* elem){
	free(elem);
}

void finalizar(){
	config_destroy(config);
	log_destroy(logs);
	list_destroy(listaDeSegmentos);
	free(parm);
	free(algoritmo);
	list_destroy_and_destroy_elements(listaDeMarcos, (void*) _marcoDestroyer);
}

void manejoError(int socket,fd_set*conjunto){
	if(kernelSocket == socket){
		printf("SERVER: Se cortó el KERNEL, NO se recibirán más conexiones\n");
		malEstado = 1;
		finalizar();
	}else{
		printf("SERVER: Se cortó un CPU socket %d \n ",socket);
		FD_CLR(socket,conjunto);
	}
}

int recibirDato(int socket, int size, void** dato,fd_set*conjunto){
	*dato = malloc(size);
	if (recv (socket, dato, size, MSG_WAITALL) <= 0){
		manejoError(socket,conjunto);
		return 0;
	}
	return 1;
}

char* devolvemeElValorDe(char* clave,t_config* config){
	if( config_has_property(config,clave) ){
		char* valor = config_get_string_value(config,clave);
		return valor;
	} else {
		printf("No existe la propiedad %s",clave);
		log_error(logs,"No existe una propiedad");
		log_destroy(logs);
		exit(1);
	}
}

bool _esLRU(char* instruccion){
	return string_equals_ignore_case( instruccion, "lru");
}

bool _esClock(char* instruccion){
	return string_equals_ignore_case( instruccion, "clock");
}

int _esPagSwap(t_pagSwap* elem){
	return (elem->numPag == buscadorSwap && elem->numSegm == segmSwap && elem->pid == pidSwap);
}

void _destructorPagSwap(t_pagSwap*elem){
	free(elem);
}

int estaSwapeada( int numPag){
	buscadorSwap = numPag;
	return list_any_satisfy(listaDePaginasSwap,(void*)_esPagSwap);
}

void inicializarEstructuras(){
	listaDeSegmentos= list_create();
	listaDeMarcos = list_create();
	listaDePaginasSwap=list_create();
	int i;
	for(i=0;i<cantidadDeMarcos;i++){
		frame_t* marcoNuevo = malloc(sizeof(frame_t));
		marcoNuevo->bitDeUso = 0;
		marcoNuevo->contadorLRU = 0;
		marcoNuevo->numero = i;
		marcoNuevo->pid = -1;
		marcoNuevo->pagina = -1;
		marcoNuevo->segmento = -1;
		list_add(listaDeMarcos,marcoNuevo);
	}
}
