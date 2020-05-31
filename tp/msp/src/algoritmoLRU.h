#include "algoritmoClock.h"

bool comparar(const void* elem1,const void* elem2){
	frame_t* x;
	frame_t* y;
	x = (frame_t*) elem1;
	y = (frame_t*) elem2;
	return (x->contadorLRU >= y->contadorLRU);
}

void _aumentarUso(frame_t* elem){
	if(elem->pagina != -1){
		elem->contadorLRU++;
	}
}

int buscarMarcoVacio(){
	int i;
	for(i=0;i< list_size(listaDeMarcos); i++){
		frame_t* marco = list_get(listaDeMarcos,i);
		if(marco->pagina == -1){
			return i;
		}
	}
	return 0;
}

frame_t* marcoMenosUsado(){
	int valor = -1;
	frame_t* marcoMenosUsado;
	int i;
	for(i=0;i<cantidadDeMarcos;i++){
		frame_t *marco=list_get(listaDeMarcos,i);
		if(valor < marco->contadorLRU){
			marcoMenosUsado=marco;
			valor = marcoMenosUsado->contadorLRU;
		}
	}
	buscadorPag=marcoMenosUsado->pagina;
	buscadorDeSegxPid = marcoMenosUsado->pid;
	buscadorDeSegxNum = marcoMenosUsado->segmento;
	segmentos_t* seg = list_find(listaDeSegmentos,(void*)_segmento);
	//MEMCOPY del marco correspondiente al contenido
	printf("\n Segmento %d pid %d pagina %d \n",seg->numero,seg->pid,marcoMenosUsado->pagina);
	fflush(stdout);
	int32_t offsetMemoria = tamanioDePagina * (marcoMenosUsado->numero);
	swapIn(seg->pid,seg->numero,marcoMenosUsado->pagina,offsetMemoria);
	log_info(logs,"Se desselecciono el marco: %d del PID %d Segmento %d Pagina %d",marcoMenosUsado->numero,marcoMenosUsado->pid,marcoMenosUsado->segmento,marcoMenosUsado->pagina);
	return marcoMenosUsado;
}

int32_t quitarPaginaMenosUsada(){
	frame_t* marcoActualizado= marcoMenosUsado();
	marcoActualizado->contadorLRU=-1;
	marcoActualizado->pagina=-1;
	marcoActualizado->segmento=-1;
	marcoActualizado->pid = -1;
	return marcoActualizado->numero;
}

int estaEnMemoria(int numpag,int *posPag,int numSegmento,int32_t pid){
	int i;
	for(i=0;i< list_size(listaDeMarcos); i++){
		frame_t* marco = list_get(listaDeMarcos,i);
		if(marco->pagina == numpag && marco->segmento == numSegmento && marco->pid == pid){
			*posPag = i;
			return 1;
		}
	}
	return 0;
}

int cantidadDeMarcosOcupados(){
	int i,marcosOcupados=0;
	for(i=0;i<cantidadDeMarcos;i++){
		frame_t* marco = list_get(listaDeMarcos,i);
		if (marco->pagina != -1){
			marcosOcupados++;
		}
	}
	return marcosOcupados;
}

int pedirPagina(int numPag,int numSegmento,int32_t pid){
	int posPagina=-1;
	frame_t* marcoActualizado;
	if(cantidadDeMarcosOcupados() == cantidadDeMarcos ){
		if(estaEnMemoria(numPag,&posPagina,numSegmento,pid)){
			marcoActualizado = list_get(listaDeMarcos,posPagina);
			marcoActualizado->contadorLRU=-1;
		}else{
			marcoActualizado = list_get(listaDeMarcos,quitarPaginaMenosUsada());
			marcoActualizado->pagina=numPag;
			marcoActualizado->contadorLRU=-1;
			marcoActualizado->pid = pid;
			marcoActualizado->segmento = numSegmento;
		}
	}else{
		if(estaEnMemoria(numPag,&posPagina,numSegmento,pid)){
			marcoActualizado = list_get(listaDeMarcos,posPagina);
			marcoActualizado->contadorLRU=0;
		}else{
			int posMarcoVacio;
			posMarcoVacio = buscarMarcoVacio();
			marcoActualizado = list_get(listaDeMarcos,posMarcoVacio);
			marcoActualizado->pagina=numPag;
			marcoActualizado->contadorLRU=-1;
			marcoActualizado->pid = pid;
			marcoActualizado->segmento = numSegmento;
		}
	}
	list_iterate(listaDeMarcos,(void*) _aumentarUso);
	return marcoActualizado->numero;
}

void _mostrarEstadoMarco(frame_t* marco){
	printf("Marco %d tiene a la pagina %d con usolru %d con segmento %d \n",marco->numero,marco->pagina,marco->contadorLRU,marco->segmento);
}

void mostrarEstadoDeUso(){
	list_iterate(listaDeMarcos,(void*) _mostrarEstadoMarco);
}
