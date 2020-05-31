#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/string.h>
#include <commons/collections/list.h>

#define KGRN  "\x1B[32m"
#define KNRM  "\x1B[0m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define PUERTO "6667"
#define BACKLOG 5
#define PACKAGESIZE 1024

char* memoriaPrincipal;
int tamMemoria = 4096;
int tamMarco = 256;
int tamArchivoARecibir;


typedef struct{
	int32_t numero;
	int32_t marco; //-2=no existe, -1=swap, 0..n= numero de marco
}pagina_t;

typedef struct{
	int32_t numero;
	int32_t pid;
	u_int32_t direccionBase;
	int32_t cantidadDePaginas;
	t_list *tablaDePaginas;
}segmentos_t;

typedef struct{
	int32_t numero;
	int32_t pagina; //si es -1 esta vacio
	int32_t segmento;
	int32_t pidSegmento;
	int32_t bitDeUso;
	int32_t contadorLRU;
}frame_t;

void imprimirCodigoCompleto(int tam,char* cod){
	int x;
	for(x=0; x < tam; x++){
		//Si es un \0, remplazalo por un '_'
		printf("%c", (cod[x] == '\0') ? '+' : cod[x]);}
	printf("\n");
}

t_list *listaDeMarcos;
t_list *listaDePaginasSwap;
t_list *listaDeSegmentos;

int contadorMarcos = 0;

int32_t hallarCantidadDePaginas(int tamanioSegmento){
	float algo = (tamanioSegmento/256);
	int cantPaginas = (int) algo;
	float resto = tamanioSegmento%256;
	if(resto != 0){
		return cantPaginas +1;
	}else{
		return cantPaginas;
	}
	return 0;
}

int memReservada = 0;

int haylugar(int tamAReservar){
	int memDisponible = tamMemoria - memReservada;
	if(memDisponible < tamAReservar){
		memReservada += tamAReservar;
		return 0;
	}
	return 1;
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

int cantidadDigitos(int numero){
	int contador;
	contador = 0;
	while(numero != 0){
		numero = numero / 10;
		contador++;
	}
	return contador;
}

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

void convertirDireccion(int numero){
	int segmento, pagina;
	int segPagina,offset;
	offset = numero % 256;
	segPagina = numero /256;
	pagina = segPagina % 4096;
	segmento = segPagina / 4096;
	printf("NUMERO EN HEXADECIMAL: %x\n\n",numero);
	printf("RESULTADO EN HEXADECIMAL:\nSegmento: %x\tPagina: %x\tOffset: %x\n\n",segmento,pagina,offset);
	printf("RESULTADO EN DECIMAL:\nSegmento: %d\tPagina: %d\tOffset: %d",segmento,pagina,offset);
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

u_int32_t crearSegmento(int32_t pid, int tamanio, int seg){
	if(haylugar(tamanio)){
		seg=255;
		segmentos_t *nodo= malloc(sizeof(segmentos_t));
		nodo->cantidadDePaginas= hallarCantidadDePaginas(tamanio);
		nodo->numero = seg;
		nodo->pid=pid;
		//TODO: asignar direccion base, no se si es necesario porque cuando implementemos los algoritmos la direccion va a cambiar
		//nodo->direccionBase = (u_int32_t) punteroDeCarga; //FIXME: revisar que este bien esta asignacion
		//punteroDeCarga += tamanio;
		nodo->tablaDePaginas=crearPaginasDelSegmento(nodo->cantidadDePaginas);
		list_add(listaDeSegmentos,nodo);
		/*u_int32_t dir = crearDireccion(nodo->numero);*///TODO: DIRECCION
		//TODO: devuelve la direccion
		/*return dir;*/
		char* c = string_new();
		char * cadZ= string_new();
		int dig;
		dig = 3 - strlen(convertirAHexa(seg));
		int i;
		if(cantidadDigitos(seg) == 0){
			dig=2;
		}
		for(i=0;i<dig;i++){
			string_append(&cadZ,"0");
		}
		string_append(&c,"0x");
		string_append(&c,cadZ);
		string_append(&c,convertirAHexa(seg));
		string_append(&c,"001ff");
		printf("\n Direccion creada %s",c);
		uint32_t dir = strtol(c,NULL,16);
		printf("\n Direccion en num %x",dir+1);// pusimos +1 para ver que sume bien , suma bien esta bien


		return 0; //por ahora dejamos asi porque sino tira error
	}
	return -1;
}

int pid = 0;
int pidDeSegmento,numeroSegmentoBuscado;

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

int _esSegmentoBuscado(segmentos_t* elem){
	return (elem->numero == numeroSegmentoBuscado && elem->pid == pidDeSegmento);
}

void imprimirTablaPaginas(int32_t pid, int numero){
	numeroSegmentoBuscado = numero;
	pidDeSegmento = pid;
	printf("PID del Segmento: %d\n", pid);
	printf("Numero del Segmento: %d\n", numero);
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
		printf("PID del Segmento alojado: %d\n",elem->pidSegmento);
		printf("Numero de Segmento Alojado: %d\n", elem->segmento);
		printf("Numero de Pagina Alojada: %d\n", elem->pagina);
	}
	printf("Bit de Uso del Marco: %d\n", elem->bitDeUso);
	printf("Instantes que no se utiliza el marco: %d\n\n", elem->contadorLRU);
}

void mostrarEstadoDeMarcos(){
	list_iterate(listaDeMarcos, (void*) impresionMarco);
}

void inicializar(){
	int cantidadDeMarcos = 16;
	listaDeSegmentos= list_create();
	listaDeMarcos = list_create();
	listaDePaginasSwap=list_create();
	int i;
	for(i=0;i<cantidadDeMarcos;i++){
		frame_t* marcoNuevo = malloc(sizeof(frame_t));
		marcoNuevo->bitDeUso = 0;
		marcoNuevo->contadorLRU = 0;
		marcoNuevo->numero = i;
		marcoNuevo->pagina = -1;
		marcoNuevo->segmento = -1;
		list_add(listaDeMarcos,marcoNuevo);
	}
}

int main(){
	inicializar();

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
	int socketCliente = accept(listenningSocket, (struct sockaddr *) &addr, &addrlen);
	printf("Cliente conectado. Esperando mensajes:\n");

	//----------------------------------------------------//
	memoriaPrincipal = malloc (sizeof(char)*tamMemoria);
	int sumadorOffset = 0;
	int segmentoUsado = 0;
	int tamTotal;
	int a = 0;

	while(a<3){
		recv(socketCliente, &tamArchivoARecibir, sizeof(int), MSG_WAITALL);
		printf("\nSe va a recibir un codigo te tamaño: %d",tamArchivoARecibir);
		/*int offset1 = tamArchivoARecibir; //para saber luego donde poner lo que me llegue*/
		int resp = 1;
		send(socketCliente,&resp,sizeof(int),0);
		char* codigo=malloc(tamArchivoARecibir);
		recv(socketCliente, codigo, tamArchivoARecibir, MSG_WAITALL);
		printf("\nEl CODIGO RECIBIDO FUE: %s\n",codigo);
		printf("\nDIR BASE DE MEMORIA: %d",(u_int32_t)memoriaPrincipal);
		printf("\nCONTENIDO MEMORIA (Antes de ingresar bytes): %s\n",memoriaPrincipal);
		printf("\nVoy a ingresar el codigo de las syscalls\n");
		/*memcpy(memoriaPrincipal,codigo,tamArchivoARecibir-1);*/

		tamTotal = tamArchivoARecibir;
		//	int tamRestante;
		//	int i;
		int offset = 0;
		int numPag = 0;
		int segmento = 0;
		crearSegmento(pid,tamTotal,segmento);
		while (tamTotal > 0){
			memcpy(memoriaPrincipal+sumadorOffset,codigo+offset,tamMarco);
			tamTotal -= tamMarco;
			sumadorOffset += tamMarco;
			offset += tamMarco;
			frame_t* marc = list_get(listaDeMarcos,contadorMarcos);
			segmentos_t* segm = list_get(listaDeSegmentos,segmentoUsado);
			marc->segmento = segmento;
			marc->pidSegmento = segm->pid;
			pagina_t* pag = list_get(segm->tablaDePaginas,numPag);
			pag->marco = marc->numero;
			marc->pagina = pag->numero;
			numPag++;
			if(numPag >= 4096){
				segmento++;
			}
			contadorMarcos++;
		}
		segmentoUsado++;
		mostrarEstadoDeMarcos();
		imprimirTablaSegmentos(pid);
		pid++;
		free(codigo);
		a++;
		send(socketCliente,&resp,sizeof(int),0);//aviso que llego bien
	}
	u_int32_t dirAMandar = strtol("0x00200A1f",NULL,16);
	u_int32_t dirRecib;
	convertirDireccion(dirAMandar);

	printf("DIR: %d\t%x",dirAMandar,dirAMandar);
//	char* algo24 = string_itoa(dirAMandar);
//	u_int32_t otraDir = strtol(algo24,NULL,0);
//	printf("OTRA DIR: %d\t%x",otraDir,otraDir);
//
//	char* cadena = string_itoa(dirAMandar);
//	int tamCad = strlen(cadena);
//	printf("TAMCAD: %d",tamCad);
//	send(socketCliente,&tamCad,sizeof(int),0);
//	sleep(1);
	send(socketCliente,&dirAMandar,sizeof(u_int32_t),0);
	recv(socketCliente,&dirRecib,sizeof(u_int32_t),MSG_WAITALL);
	convertirDireccion(dirRecib);
	printf("\nCONTENIDO MEMORIA: \n");//TODO: la msp lee de a 1 en for desde i hasta tamaño PEDIDO.
	tamTotal = sumadorOffset;
	sumadorOffset = 0;
	int cont = 0;
	while (tamTotal > 0){
		char* marco = malloc(tamMarco);
		memcpy(marco,memoriaPrincipal+sumadorOffset,tamMarco);
		printf("\nMarco: %d Contenido: \n",cont);
		imprimirCodigoCompleto(tamMarco,marco);
		tamTotal -= tamMarco;
		sumadorOffset += tamMarco;
		cont++;
		free(marco);
	}




	//----------------PRUEBA CPU------------------//



	close(socketCliente);
	close(listenningSocket);
	free(memoriaPrincipal);
	return 0;
}

/*	while(tamTotal > 0){
		tamRestante = tamMarco;
		printf("tamTotal %d\n",tamTotal);
		for(i = 0;i<tamMarco+1;i++){
			if (codigo[i+2]=='\0'){
				if(tamRestante > 6){
					memcpy(memoriaPrincipal+sumadorOffset,codigo+sumadorOffset,6);
					sumadorOffset += 6;
					tamRestante -= 6;
					tamTotal -= 6;
				}else{
					sumadorOffset += tamRestante;
					tamRestante = 0;
				}

			}else{
				if(tamRestante > 1){
					memcpy(memoriaPrincipal+sumadorOffset,codigo+sumadorOffset,1);
					sumadorOffset++;
					tamRestante--;
					tamTotal--;
				}else {
					sumadorOffset += tamRestante;
					tamRestante = 0;
				}
			}

		}
	}*/
/*char cadParaImprimir2[4];
memcpy(cadParaImprimir2,memoriaPrincipal+offset1-1+6,4);*/
/*//Ahora estamos probando el codigo del STDIN.bc donde hace LOAD A, #cadena y la cadena es hola
//lo que hace es que te devuelve la direccion donde ir a buscar la cadena que precisas guardar
//entonces lo que hacemos aca es con la cadParaImprimir2[3] (o sea la direccion que te da, porque es (0)(0)(0)(65))
//y copiamos el contenido de la memoria de la posicion donde comienza el segundo codigo + cadParaImprimir2[3]
//en otra variable y la imprimimos para que se imprima la cadena, no se como es la verificacion de si es
//cadena o un numero lo que te devuelve...
printf("nueva dir: %d",cadParaImprimir2[3]);
char cadParaImprimir3[4];
memcpy(cadParaImprimir3,memoriaPrincipal+offset1-1+cadParaImprimir2[3],4);
imprimirCodigoCompleto(4, cadParaImprimir3);
//printf("\nR: %d",cadParaImprimir2[0]);*/




