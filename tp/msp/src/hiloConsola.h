#include "algoritmoLRU.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BACKLOG 5

char* conversorHexa(int num){
	char* convertir = string_new();
	if(num < 10){
		convertir = string_itoa(num);
	}else if(num == 10){
		convertir = "A";
	}else if(num == 11){
		convertir = "B";
	}else if(num == 12){
		convertir = "C";
	}else if(num == 13){
		convertir = "D";
	}else if(num == 14){
		convertir = "E";
	}else if(num == 15){
		convertir = "F";
	}
	return convertir;
}

void convertirDireccion(int numero, direccion_t* dir){
	int segmento, pagina;
	int segPagina,offset;
	offset = numero % 256;
	segPagina = numero /256;
	pagina = segPagina % 4096;
	segmento = segPagina / 4096;
	dir->segmento = segmento;
	dir->pagina = pagina;
	dir->offset = offset;
}

char* convertirAHexa(int num){
	char* algo1 = string_new();
	char* algo2 = string_new();
	int cociente =num/16;
	int resto = num%16;
	if(cociente < 16){
		string_append(&algo1,conversorHexa(cociente));
	}else{
		convertirAHexa(cociente);
	}
	string_append(&algo2,conversorHexa(resto));
	char* algo = string_new();
	string_append(&algo,algo1);
	string_append(&algo,algo2);
	return algo;
}

int32_t hallarCantidadDePaginas(int tamanioSegmento){
	float algo = tamanioSegmento/tamanioDePagina;
	int cantPaginas = (int) algo;
	float resto = tamanioSegmento%tamanioDePagina;
	if(resto != 0){
		return cantPaginas +1;
	}else{
		return cantPaginas;
	}
	return 0;
}

t_list * crearPaginasDelSegmento(int32_t cantPag){
	t_list *paginasDelSegmento = list_create();
	int i;
	for(i=0;i<cantPag;i++){
		pagina_t* pagina = malloc(sizeof(pagina_t));
		pagina->numero=i;
		pagina->marco=-2;
		list_add(paginasDelSegmento,pagina);
	}
	return paginasDelSegmento;
}

int haylugar(int32_t cantidadPag){
	int swappLibre = (cantMaxDePaginasSwapeadas - cantidadArchivosSwapeados) * tamanioDePagina;
	int tamAReservar = tamanioDePagina*cantidadPag;
	int32_t memDisponible = memMaxima - memReservada;
	if( (memDisponible + swappLibre) > tamAReservar){
		if (memDisponible >= tamAReservar){
			memReservada += tamAReservar;
		}else{
			log_info(logs,"La memoria principal esta llena");
		}
		return 1;
	}
	log_info(logs,"La memoria principal esta llena al igual que el espacio de intercambio");
	return 0;
}

void _busquedaProximoNumeroDeSegmento(segmentos_t* elem){
	if (elem->pid == pidDeSegmentoBuscado){
		contadorDeSegmentos++;
	}
}
int hallarValorSegmento(int pid){
	pidDeSegmentoBuscado = pid;
	contadorDeSegmentos = 0;
	list_iterate(listaDeSegmentos,(void*)_busquedaProximoNumeroDeSegmento);
	return contadorDeSegmentos;
}

int cantidadDigitos(int numero){
	int contador;
	contador = 0;
	while(numero != 0){
		numero = numero / 10;
		contador++;
	}
	return contador;
}

u_int32_t crearDireccion(int seg){
	char* c = string_new();
	char * cadZ= string_new();
	int dig;
	dig = 3 - strlen(convertirAHexa(seg));
	int i;
	if(seg == 0){
		dig=1;
	}
	for(i=0;i<dig;i++){
		string_append(&cadZ,"0");
	}
	string_append(&c,"0x");
	string_append(&c,cadZ);
	string_append(&c,convertirAHexa(seg));
	string_append(&c,"00000");
	uint32_t dir = strtol(c,NULL,16);
	return dir;
}

u_int32_t crearSegmento(int32_t pid, int tamanio){
	int puedeReservar = haylugar(hallarCantidadDePaginas(tamanio));
	if(puedeReservar){
		segmentos_t *nodo= malloc(sizeof(segmentos_t));
		nodo->cantidadDePaginas= hallarCantidadDePaginas(tamanio);
		nodo->numero = hallarValorSegmento(pid);
		nodo->pid=pid;
		nodo->tablaDePaginas=crearPaginasDelSegmento(nodo->cantidadDePaginas);
		list_add(listaDeSegmentos,nodo);
		u_int32_t dir = crearDireccion(nodo->numero);
		return dir;
	}
	return -1;
}

bool esigual(void*elem){
	segmentos_t* x = (segmentos_t*) elem;
	return (x->pid == pidADestruir);
}

void _destructorPaginas(pagina_t* elem){
	free(elem);
}

void _destructorSegmentos(segmentos_t* elem){
	free(elem);
}

int destruirSegmento(int32_t pid, u_int32_t base){
	direccion_t* dir = malloc(sizeof(direccion_t));
	convertirDireccion(base,dir);
	pidDeSegmento = pid;
	numeroSegmentoBuscado = dir->segmento;
	segmentos_t *nodoAEliminar= list_find(listaDeSegmentos,(void*) _esSegmentoBuscado);
	if(nodoAEliminar == NULL){
		return -1;
	}
	char* contenido= malloc(tamanioDePagina);// es para reutilizar swap out
	int i;
	for(i=0;i<nodoAEliminar->cantidadDePaginas;i++){
		pagina_t* pagina = list_get(nodoAEliminar->tablaDePaginas,i);
		if(pagina->marco==-1){//esta swapeado
			swapOut(nodoAEliminar->pid,nodoAEliminar->numero,pagina->numero,contenido);
		}else if(pagina->marco>=0){//esta en memoria
			frame_t* frame = list_get(listaDeMarcos,pagina->marco);
			frame->pid = -1;
			frame->pagina = -1;
			frame->segmento = -1;
			frame->bitDeUso = 0;
			frame->contadorLRU = 0;
			memReservada -= tamanioDePagina;
		}
	}
	list_destroy_and_destroy_elements(nodoAEliminar->tablaDePaginas,(void*)_destructorPaginas);
	list_remove_and_destroy_by_condition(listaDeSegmentos,(void*) _esSegmentoBuscado,(void*)_destructorSegmentos);
	free(contenido);
	free(dir);
	return 0;
}

int pedirMarcoAlgoritmo(int nroPagina,int numSegmento, int32_t pid){
	int marco;
	if (_esLRU(algoritmo)){
		marco = pedirPagina(nroPagina,numSegmento,pid);
	}else{
		marco = pedirPaginaClock(nroPagina,numSegmento,pid);
	}
	return marco;
}

u_int32_t solicitarMemoria(int32_t pid, u_int32_t dirLogica, int tamanio){
	direccion_t* dir = malloc(sizeof(direccion_t));
	convertirDireccion(dirLogica,dir);
	pidDeSegmento = pid;
	numeroSegmentoBuscado = dir->segmento;
	segmentos_t *nodo= list_find(listaDeSegmentos,(void*) _esSegmentoBuscado);
	if(nodo == NULL){
		return -1;
	}
	pagina_t* pag = list_get(nodo->tablaDePaginas,dir->pagina);
	if(pag == NULL){
		return -1;
	}
	int numMarco;
	if(pag->marco == -1){
		char* contenido = malloc(tamanioDePagina+1);
		contenido = swapOut(pid,dir->segmento,dir->pagina,contenido);
		numMarco = pedirMarcoAlgoritmo(pag->numero,nodo->numero,nodo->pid);
		u_int32_t offsetMemoria = numMarco*tamanioDePagina;
		memcpy(memoriaPrincipal+offsetMemoria,contenido,tamanioDePagina);
		log_info(logs,"Se selecciono el marco: %d para el PID %d Segmento %d Pagina %d",numMarco,nodo->pid,nodo->numero,pag->numero);
		free(contenido);
		pag->marco = numMarco;
	}else if(pag->marco >= 0){
		numMarco = pag->marco;
		//Lo actualizamos
		frame_t* marco = list_get(listaDeMarcos,numMarco);
		marco->bitDeUso = 1;
		marco->contadorLRU = 0;
	}else if(pag->marco == -2){
		return -1;
	}
	u_int32_t offsetMemoria = tamanioDePagina*(numMarco) + dir->offset;
	return offsetMemoria;
}

int escribirMemoria(int32_t pid, u_int32_t dirLogica, void* bytesAEscribir, int tamanio){
	direccion_t* dir = malloc(sizeof(direccion_t));
	convertirDireccion(dirLogica,dir);
	pidDeSegmento = pid;
	numeroSegmentoBuscado = dir->segmento;
	segmentos_t *nodo= list_find(listaDeSegmentos,(void*) _esSegmentoBuscado);
	if(nodo == NULL){
		printf("\nno existe segmento: %d\n",dir->segmento);
		return -1;
	}
	int i,x,y;
	x = 0;
	y = 0;
	for( i = 0; i < nodo->cantidadDePaginas ; i++ ){
		pagina_t* pag = list_get(nodo->tablaDePaginas,dir->pagina+i);
		if(pag == NULL){
			printf("no existe pagina: %d\n",dir->pagina+i);
			return -1;
		}
		int numMarco;
		if(pag->marco < 0){
			//pedir marco para la pagina al algoritmo y guardarlo en la variable numMarco
			numMarco = pedirMarcoAlgoritmo(pag->numero,nodo->numero,nodo->pid);
			log_info(logs,"Se selecciono el marco: %d para el PID %d Segmento %d Pagina %d",numMarco,nodo->pid,nodo->numero,pag->numero);
			if(pag->marco == -1){
				char* contenido = malloc(tamanioDePagina+1);
				contenido = swapOut(nodo->pid,nodo->numero,pag->numero,contenido);
				int32_t offsetMem = tamanioDePagina*numMarco;
				memcpy(memoriaPrincipal+offsetMem,contenido,tamanioDePagina);
			}
			pag->marco = numMarco;
		}else{
			numMarco = pag->marco;
			//Lo actualizamos
			frame_t* marco = list_get(listaDeMarcos,numMarco);
			marco->bitDeUso = 1;
			marco->contadorLRU = 0;
		}
		u_int32_t offsetMemoria = (tamanioDePagina* numMarco) + dir->offset;
		if(tamanioDePagina<tamanio){
			memcpy(memoriaPrincipal+offsetMemoria,bytesAEscribir+x,y = tamanioDePagina-dir->offset);
			x += y;
			dir->offset = 0;
			tamanio -= tamanioDePagina;
		}else{
			memcpy(memoriaPrincipal+offsetMemoria,bytesAEscribir+x,y = tamanio);
			break;
		}
	}
	return 0;
}

void* hiloConsola(){
	printf("\nConsola de la MSP\n");
	int eleccion, tamanio, nroSegmento;
	int32_t pid;
	u_int32_t dirLogica;
	segmentos_t* segm;
	char* bytes = string_new();
	char* str = string_new();
	char* str2 = string_new();
	char* str3 = string_new();
	while(1) {
		etiqueta2:
		printf("\n\n\t1. Crear Segmento\n");
		printf("\t2. Destruir Segmento\n");
		printf("\t3. Escribir en Memoria\n");
		printf("\t4. Leer de Memoria\n");
		printf("\t5. Imprimir Tabla de Segmentos\n");
		printf("\t6. Imprimir Tabla de Paginas\n");
		printf("\t7. Imprimir el estado de los Marcos\n");
		printf("\t8. Salir\n");
		printf("Ingrese el numero de la opcion elegida:");
		scanf("%s",str);
		eleccion = atoi(str);
		if(eleccion == atoi("")){
			printf("Ingreso invalido\n");
			goto etiqueta2;
		}else{
			switch(eleccion){
			case 1:
				printf("Ingrese un pid:");
				scanf("%s",str3);
				pid = atoi(str3);
				printf("Ingrese un tamaño:");
				scanf("%s",str2);
				tamanio = atoi(str2);
				dirLogica = crearSegmento(pid, tamanio);
				printf("Esta es la direccion base del segmento que le corresponde a ese pid: %d", dirLogica);
				break;
			case 2:
				printf("Ingrese un pid:");
				scanf("%s",str3);
				pid = atoi(str3);
				printf("Ingrese la direccion base correspondiente:");
				scanf("%s",str2);
				dirLogica = atoi(str2);
				destruirSegmento(pid, dirLogica);
				break;
			case 3:
				printf("Ingrese un pid:");
				scanf("%s",str3);
				pid = atoi(str3);
				printf("Ingrese la direccion base correspondiente:");
				scanf("%s",str2);
				dirLogica = atoi(str2);
				printf("Ingrese la cadena que desea escribir:");
				scanf("%s",bytes);
				printf("Ingrese un tamaño:");
				scanf("%s",str2);
				tamanio = atoi(str2);
				int estado = escribirMemoria(pid, dirLogica, bytes, tamanio);
				if(estado == 0){
					printf("El segmento se escribió correctamente\n");
				}else{
					printf("Hubo un error al escribir el segmento\n");
					printf("ABORTANDO... \n" KWHT);
					exit(0); //Aborta por Seg.Fault.
				}
				break;
			case 4:
				printf("Ingrese un pid:");
				scanf("%s",str3);
				pid = atoi(str3);
				printf("Ingrese la direccion base correspondiente:");
				scanf("%s",str2);
				dirLogica = atoi(str2);
				printf("Ingrese un tamaño:");
				scanf("%s",str2);
				tamanio = atoi(str2);
				u_int32_t rta = solicitarMemoria(pid, dirLogica, tamanio);
				char* var = malloc(tamanio+1);
				memcpy(var,memoriaPrincipal+rta,tamanio);
				var[tamanio] = '\0';
				printf("El contenido de la memoria segun los datos ingresados es:\n");
				int j;
				for (j=0;j<tamanio;j++){
					printf("%c,",var[j]);
				}
				break;
			case 5:
				errorPID:
				printf("Ingrese un pid:");
				scanf("%s",str3);
				pid = atoi(str3);
				pidBuscado = pid;
				segm = list_find(listaDeSegmentos, (void*) esPidBuscado);
				if(segm == NULL){
					printf("Ha ingresado un PID inválido\n");
					printf("Desea reingresarlo? (Si / No)\n");
					scanf("%s",str);
					if(string_equals_ignore_case(str,"si")){
						goto errorPID;
					}else{
						break;
					}
				}
				imprimirTablaSegmentos(pid);
				break;
			case 6:
				errorPIDySegm:
				printf("Ingrese un pid:");
				scanf("%s",str3);
				pid = atoi(str3);
				printf("Ingrese un numero de segmento:");
				scanf("%s",str2);
				nroSegmento = atoi(str2);
				numeroSegmentoBuscado = nroSegmento;
				pidDeSegmento = pid;
				segm = list_find(listaDeSegmentos, (void*) _esSegmentoBuscado);
				if(segm == NULL){
					printf("Ha ingresado un PID o un Numero de Segmento invalido\n");
					printf("Desea reingresarlos? (Si / No)\n");
					scanf("%s",str);
					if(string_equals_ignore_case(str,"si")){
						goto errorPIDySegm;
					}else{
						break;
					}
				}
				imprimirTablaPaginas(pid,nroSegmento);
				break;
			case 7:
				mostrarEstadoDeMarcos();
				break;
			case 8:
				break;
			default:
				printf("\nIngrese una opcion valida\n");
				break;
			}
			if(eleccion==8){
				free(str);
				free(str2);
				free(str3);
				free(bytes);
				break;
			}
		}
	}
	pthread_exit(NULL);
}
