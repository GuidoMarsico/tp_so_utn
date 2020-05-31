#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/string.h>

#define IP "127.0.0.1"
#define PUERTO "6667"
#define PACKAGESIZE 1024

char* codigo;
uint32_t size;
FILE *binario;
int tamArchivo;

void imprimirCodigoCompleto(int tam,char* cod){
	printf("\nEl código es: \n");
	int x;
	for(x=0; x < tam; x++){
		//Si es un \0, remplazalo por un '_'
		printf("%c", (cod[x] == '\0') ? '+' : cod[x]);}
	printf("\n");
}

void levantarCodigo(int tam, char* cod, char* path){

	if ((binario = fopen(path, "r")) == NULL){
		printf("\n No existe la ruta\n");
		exit(0);
	}else{
		fseek(binario, 0, SEEK_END); // puntero a la posicion final del binario
		size = ftell(binario); // obtengo el tamaño del script
		fseek(binario, 0, SEEK_SET); //devuelvo el puntero a la primera línea
		codigo = malloc(sizeof(char)*(size +1));
		if(codigo) { //debería hacer un checkeo de error por las dudas
			fread(codigo, sizeof(char), size, binario);
			codigo[size] = '\0';
		}
	}
	tamArchivo = size+1;
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

int main(){

	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(IP, PUERTO, &hints, &serverInfo);
	int serverSocket;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);
	printf("Conectado\n");

//---------LEVANTO SYSCALLS-------------------//
	levantarCodigo(tamArchivo,codigo,"/home/utnso/git/tp-2014-2c-rafagadeterror/pruebasConMSP/kernelM/systemCalls.bc");
	printf("\nTAM: %d CODIGO: %s\n",tamArchivo,codigo);
	printf("\nTamaño con strlen: %d",strlen(codigo));
//---------------MANDO SYSCALLS------------------------//
	int num;
	send(serverSocket,&tamArchivo,sizeof(int),0);//mando tamaño del codigo
	recv(serverSocket,&num,sizeof(int),MSG_WAITALL);//respuesta de la MSP que le llego bien
	send(serverSocket,codigo,tamArchivo,0);//le mando el codigo
	imprimirCodigoCompleto(tamArchivo,codigo);
	free(codigo);
//---------LE MANDO OTRO CODIGO---------------//
	levantarCodigo(tamArchivo,codigo,"/home/utnso/git/tp-2014-2c-rafagadeterror/pruebasConMSP/kernelM/hola.bc");
	printf("\nSe le enviará un segundo código, de tamaño %d\n",tamArchivo);
	printf("\nTamaño con strlen: %d\n",strlen(codigo));
	recv(serverSocket,&num,sizeof(int),MSG_WAITALL);
	send(serverSocket,&tamArchivo,sizeof(int),0);//mando tamaño del codigo
	recv(serverSocket,&num,sizeof(int),MSG_WAITALL);//respuesta de la MSP que le llego bien
	send(serverSocket,codigo,tamArchivo,0);//le mando el codigo
	imprimirCodigoCompleto(tamArchivo,codigo);
	free(codigo);

//---------LE MANDO OTRO CODIGO---------------//
	levantarCodigo(tamArchivo,codigo,"/home/utnso/git/tp-2014-2c-rafagadeterror/pruebasConMSP/kernelM/out.bc");
	printf("\nSe le enviará un segundo código, de tamaño %d\n",tamArchivo);
	printf("\nTamaño con strlen: %d\n",strlen(codigo));
	recv(serverSocket,&num,sizeof(int),MSG_WAITALL);
	send(serverSocket,&tamArchivo,sizeof(int),0);//mando tamaño del codigo
	recv(serverSocket,&num,sizeof(int),MSG_WAITALL);//respuesta de la MSP que le llego bien
	send(serverSocket,codigo,tamArchivo,0);//le mando el codigo
	imprimirCodigoCompleto(tamArchivo,codigo);
	free(codigo);


	u_int32_t dirRecib,algo353;
	recv(serverSocket,&algo353,sizeof(u_int32_t),MSG_WAITALL);//esto esta aca porque la msp hace un send al final de cada codigo que recibe
	recv(serverSocket,&dirRecib,sizeof(u_int32_t),MSG_WAITALL);
	printf("DIR RECIB: %x",dirRecib);
	dirRecib += 1;
	send(serverSocket,&dirRecib,sizeof(u_int32_t),0);


	close(serverSocket);
	return 0;

}
