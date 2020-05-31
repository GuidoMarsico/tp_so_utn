#include "BiblioSwap.h"

int punteroClock=0;
int buscadorPag,buscadorPaginaConSegmento;
int32_t buscadorDeSegxPid;
int buscadorDeSegxNum;

int _segmento (segmentos_t* elem){
	return (elem->numero == buscadorDeSegxNum && elem->pid == buscadorDeSegxPid);
}

int estaEnMemoriaClock(int numPag, int* pos, int nroSegmento, int32_t pid){
	int i;
	for(i=0;i<cantidadDeMarcos;i++){
		frame_t* marco = list_get(listaDeMarcos,i);
		if(marco->pagina== numPag && marco->segmento == nroSegmento && marco->pid == pid){
			*pos = i;
			return 1;
		}
	}
	return 0;
}

int colocarPaginaClock(int numPag,int numSegmento, int32_t pid){
	int i;
	int num = 0;
	int guardo=1;
	for(i=punteroClock;i<cantidadDeMarcos;i++){
		frame_t* marco = list_get(listaDeMarcos,i);
		if(marco->bitDeUso == 0){
			if(marco->pagina !=-1){
				buscadorPag=marco->pagina;
				buscadorDeSegxPid = marco->pid;
				buscadorDeSegxNum = marco->segmento;
				segmentos_t* seg = list_find(listaDeSegmentos,(void*)_segmento);
				//MEMCOPY del marco correspondiente al contenido
				int32_t offsetMemoria = tamanioDePagina * (marco->numero);
				swapIn(seg->pid,seg->numero,marco->pagina,offsetMemoria);
				log_info(logs,"Se desselecciono el marco: %d del PID %d Segmento %d Pagina %d",marco->numero,marco->pid,marco->segmento,marco->pagina);
			}
			marco->bitDeUso=1;
			marco->pagina=numPag;
			marco->segmento = numSegmento;
			marco->pid = pid;
			guardo=0;
			punteroClock=i+1;
			if(punteroClock >= cantidadDeMarcos){
				punteroClock=0;
			}
			return marco->numero;
		}else{
			marco->bitDeUso=0;
		}
	}
	if(guardo){
		punteroClock=0;
		num = colocarPaginaClock(numPag,numSegmento,pid);
	}
	return num;
}

int pedirPaginaClock(int numPag,int numSegmento,int32_t pid){
	frame_t* marco;
	int numMarco;
	int posicionEncontrada;
	if(estaEnMemoriaClock(numPag,&posicionEncontrada,numSegmento,pid)){
		marco = list_get(listaDeMarcos,posicionEncontrada);
		marco->bitDeUso=1;
		numMarco = marco->numero;
	}else{
		numMarco = colocarPaginaClock(numPag,numSegmento,pid);
	}
	return numMarco;
}

void _mostrarEstadoMarcoClock(frame_t* marco){
	printf("Marco %d tiene a la pagina %d con bitDeUso %d con segmento %d \n",marco->numero,marco->pagina,marco->bitDeUso,marco->segmento);
}

void mostrarEstadoDeUsoClock(){
	list_iterate(listaDeMarcos,(void*) _mostrarEstadoMarcoClock);
	printf("Puntero Clock apunta al marco %d\n",punteroClock + 1);
}
