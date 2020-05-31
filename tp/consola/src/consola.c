#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <commons/log.h>
#include <commons/config.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <commons/string.h>
#include <string.h>
#define KGRN  "\x1B[32m"
#define KNRM  "\x1B[0m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

//---Para Archivo de configuracion---//
#define refConsolaConfig "/home/utnso/git/tp-2014-2c-rafagadeterror/tp/consola/consola.conf"

typedef struct{
	char* ipKernel;
	char* puertoKernel;
}thread_parm_t;

int socketKernel;

int recibir(int size, void** dato){
	*dato = malloc(size);
	if (recv (socketKernel, dato, size, MSG_WAITALL) <= 0){
		printf("\n Se cerro el KERNEL abortando ... \n");
		exit(0);
	}
	return 1;
}
char* devolvemeElValorDe(char* clave,t_config* config){
	if( config_has_property(config,clave) ){
		char* valor = config_get_string_value(config,clave);
		return valor;
	} else {
		printf("No existe la propiedad %s",clave);
		exit(1);
	}
}

typedef struct {
	int tamCod;
	char* codigo;
}socketStruct;

int conectarCliente(char *ip, char* port,t_log *logs){
	int s;
	struct sockaddr_in dir;
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0){
		log_error(logs, "No se pudo crear el socket");
		return -1;
	}
	dir.sin_family = AF_INET;
	dir.sin_port = htons(atoi(port));
	dir.sin_addr.s_addr = inet_addr(ip);

	if (connect(s, (struct sockaddr*) &dir, sizeof(dir))== -1) {
		log_error(logs, "Error al conectar con el Kernel");
		close(s);
		exit(0);
	}
	return s;
}


int main(int argc, char *argv[]) {


	t_log* logs;


	logs = log_create("logConsola", "consola.c", 1, LOG_LEVEL_TRACE); //LOG

	thread_parm_t *parm = NULL;
	parm = malloc(sizeof(thread_parm_t));

	t_config* config = config_create(refConsolaConfig);

	parm->ipKernel = devolvemeElValorDe("IPKERNEL",config);
	parm->puertoKernel = devolvemeElValorDe("PUERTOKERNEL",config);

	printf(KBLU "\nLa configuración de la consola es:");
	printf("\nIpKernel:%s\tPuertoKernel:%s\n\n" KWHT,parm->ipKernel,parm->puertoKernel);


	socketKernel = conectarCliente(parm->ipKernel, parm->puertoKernel, logs);

	//---ABRE BINARIO, TOMA EL TAMAÑO Y GUARDA SU CONTENIDO EN "CODIGO"
	FILE *binario;
	int j;
	uint32_t size;
	char* codigo;
	if (argc > 1){
		for (j = 1; j < argc; j++){
			if ((binario = fopen(argv[j], "r")) == NULL){ //HAY QUE FIJARNOS DESPUES COMO LO PASAMOS, Y SI ESTA ES LA FORMA ES ABRIRLO
				perror(argv[j]);
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
		}
	}

	//-----------------------------------------------------------------------------//
	printf("\nEl tamaño del binario es: %d\n",size);
	printf("\nSu codigo:");
	printf("\n%s\n",codigo);

	socketStruct* infoParaKernel = malloc(sizeof(socketStruct));

	infoParaKernel->tamCod = size;
	infoParaKernel->codigo = codigo;
	send(socketKernel,&infoParaKernel->tamCod,sizeof(int),0);
	int respuesta;
	recibir(sizeof(int),(void*)&respuesta);
	if (respuesta==1){
		//manda codigo
		send(socketKernel,infoParaKernel->codigo,infoParaKernel->tamCod,0);
	}else{
		log_error(logs,"No se pudo reservar memoria");
		fflush(stdout);
		exit(0);
	}
	//se queda esperando por peticiones o errores
	int mensaje;

	while(1){

		if(recibir(sizeof(int),(void*)&mensaje) > 0){
			if(mensaje==1){
				//Entrada por teclado de NUM
				char * str = string_new();
				int32_t numero = 0;
				int32_t min = -2147483648;
				int32_t max = 2147483647;

				printf("\nIngrese un número entre -2.147.483.648 y 2.147.483.647");
				loco:
				scanf("%s",str);
				numero = atoi(str);
				if(numero <= min|| numero >= max || numero==atoi("")){	//TODO: PROBAR SI ANDA EN PROY A PARTE
					printf("\nIngrese un número válido..\n");
					goto loco;
				}
				send(socketKernel,&numero,sizeof(int32_t),0);
				free(str);

			}else if(mensaje == 2){//Entrada por teclado de CADENA
				//PIDE TAMAÑO CADENA
				int num = 1;
				char* cadena = string_new();
				int32_t tamCadena;

				send(socketKernel,&num,sizeof(int),0);
				recibir(sizeof(int32_t),(void*)&tamCadena);
				printf("\nIngrese una cadena no más larga que %d caracteres",tamCadena);	//TODO: PROBAR SI ANDA EN PROY A PARTE
				loco2:
				scanf("%s",cadena);
				if(string_is_empty(cadena) || strlen(cadena)>tamCadena){
					printf("\nIngrese cadena válida..\n");
					goto loco2;
				}
				send(socketKernel,cadena,strlen(cadena)+1,0);
				free(cadena);

			}else if(mensaje == 3){//IMPRESIÓN POR PANTALLA NUM
				//PEDIR NUMERO A IMPRIMIR
				int num = 1;
				int n;
				send(socketKernel,&num,sizeof(int),0);
				recibir(sizeof(int),(void*)&n);
				printf("\nEl número recibido fue: %d\n",n);

			}else if(mensaje == 4){//IMPRESIÓN POR PANTALLA	CADENA
				//PIDE TAMAÑO DE CADENA A RECIBIR
				//PEDIR CADENA A IMPRIMIR
				int num = 1;
				int n;
				send(socketKernel,&num,sizeof(int),0);
				recibir(sizeof(int),(void*)&n);
				char cad[n];
				send(socketKernel,&num,sizeof(int),0);
				recibir(n+1,(void*)&cad);
				printf("\nLa cadena recibida fue: %s",cad);

			}else if (mensaje == 5){//ERROR
				int opcion;
				int tamanioRecibido;
				algo:
				if((tamanioRecibido = recibir(sizeof(int),(void*)&opcion))<=0){
					goto algo;
					printf("tamanioRecibido: %d\n",tamanioRecibido);
				}
				printf("tamanioRecibido: %d\n",tamanioRecibido);
				printf("opcion: %d\n", opcion);
				if(opcion==0){
					printf("Segmentation fault: no se pudo eliminar de memoria");
				}else if(opcion == 1){
					printf("Hubo un problema con un CPU que ejecutaba HILO KERNEL. ABORTANDO....");
					exit(0);
				}else if(opcion == 2){
					printf("Se interrumpio CPU");
				}else if(opcion == 3){
					printf("Segmentation fault: no se pudo leer de memoria");
				}else if(opcion == 4){
					printf("No se pudo crear segmento stack para el hilo hijo. Abortando proceso");
				}else if(opcion == 5){
					printf("Aborto la CPU por la lectura de una instruccion inválida");
				}else if(opcion == 6){
					printf("Se superó el límite para asignación de memoria (Memory Overload) - No se pudo crear segmento");
				}else if(opcion == 7){
					printf("Se generó un error al intentar dividir por cero... Se abortó el programa en ejecucion");
				}
				fflush(stdout);

			}else if (mensaje == 6){
				printf("\nTerminó la ejecución de todos los procesos\n");
				exit(0);
			}
		}
	}
	close(socketKernel);
	log_destroy(logs);
	free(parm);
	return EXIT_SUCCESS;

}
