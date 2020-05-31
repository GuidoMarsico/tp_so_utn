#include "globalesMSP.h"

void armarNombreDeArchivo(int pid,int segmento,int pagina, char* nombreDeArchivo){
	char *nombre1= string_itoa(pid);
	char *nombre2= string_itoa(segmento);
	char *nombre3= string_itoa(pagina);
	string_append(&nombreDeArchivo,nombre1);
	string_append(&nombreDeArchivo,nombre2);
	string_append(&nombreDeArchivo,nombre3);
	string_append(&nombreDeArchivo,".bc");
	return;
}

char* swapOut(int pid,int segmento,int pagina ,char* contenido){
	char *nombreDeArchivo= string_new();
	FILE *archivo;
	armarNombreDeArchivo(pid,segmento,pagina,nombreDeArchivo);
	archivo= fopen (nombreDeArchivo, "rb");
	if (archivo == NULL){
		log_error(logs,"No se puede abrir el archivo de la pagina buscada, o no existe el archivo");
		return NULL;
	}else{
		fread(contenido, sizeof(char), tamanioDePagina, archivo);
		contenido[tamanioDePagina] = '\0';
		/*int j;
		for(j=0;j<tamanioDePagina;j++){
			contenido[j] = fgetc(archivo);
		}
		fgets(contenido,tamanioDePagina+1,archivo);*/
		remove(nombreDeArchivo);
		buscadorSwap=pagina;
		segmSwap=segmento;
		pidSwap= pid;
		list_remove_and_destroy_by_condition(listaDePaginasSwap,(void*)_esPagSwap,(void*)_destructorPagSwap);
		cantidadArchivosSwapeados--;
		printf(KCYN);
		log_info(logs,"Se sacó del espacio de intercambio: la pagina %d del Segmento %d del PID %d",pagina,segmento,pid);
		printf(KWHT);
		free(nombreDeArchivo);
	}
	return contenido;
}

void eliminarPaginaSwappeada(t_pagSwap* elem){
	char *nombreDeArchivo= string_new();
	armarNombreDeArchivo(elem->pid,elem->numSegm,elem->numPag,nombreDeArchivo);
	remove(nombreDeArchivo);
}

void eliminarTodoLoSwappeado(){
	list_iterate(listaDePaginasSwap,(void*)eliminarPaginaSwappeada);
	list_destroy_and_destroy_elements(listaDePaginasSwap,(void*)_destructorPagSwap);
}

void swapIn(int pid,int segmento,int pagina ,int32_t offsetMemoria){
	if(cantidadArchivosSwapeados>=cantMaxDePaginasSwapeadas){
		log_error(logs,"No se puede swappear porque se excedio el espacio de intercambio");
	}else{
		char *nombreDeArchivo= string_new();
		FILE *archivo;
		armarNombreDeArchivo(pid,segmento,pagina,nombreDeArchivo);
		archivo= fopen (nombreDeArchivo, "wb");
		if (archivo==NULL){
			log_error(logs,"No se puede crear un archivo");
		}
		char* contenido= malloc(tamanioDePagina);
		memcpy(contenido,memoriaPrincipal+offsetMemoria,tamanioDePagina);
		/*int j;
		for(j=0;j<tamanioDePagina;j++){
			fputc(contenido[j], archivo);
		}*/
		fwrite(contenido,sizeof(char),tamanioDePagina,archivo);
		free(contenido);
		t_pagSwap* paginaSwap= malloc(sizeof(t_pagSwap));
		paginaSwap->numPag= pagina;
		paginaSwap->numSegm = segmento;
		paginaSwap->pid = pid;
		list_add(listaDePaginasSwap,paginaSwap);
		numeroSegmentoBuscado = segmento;
		pidDeSegmento = pid;
		segmentos_t* nodo= list_find(listaDeSegmentos, (void*) _esSegmentoBuscado);
		int i;
		for(i=0;i<nodo->cantidadDePaginas;i++){
			paginaBuscadaSwap = pagina;
			list_iterate(nodo->tablaDePaginas,(void*) _actualizarPaginaSwapp);
		}
		cantidadArchivosSwapeados++;
		printf(KCYN);
		log_info(logs,"Se introdujo al espacio de intercambio: la pagina %d del Segmento %d del PID %d",pagina,segmento,pid);
		printf(KWHT);
		fclose(archivo);
		free(nombreDeArchivo);
	}
	return;
}
