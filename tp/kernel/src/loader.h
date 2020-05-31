#include "colasKernel.h"

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
//		memcpy(contenido, contenidoRecibido, tamanio);
	}
	return contenido;
}

u_int32_t reservarDireccion(int tamanio, int32_t pid){
	u_int32_t dir;
	int opcion = 1;
	send(socketMSP, &opcion, sizeof(int), 0);
	reservar_destruir_enviar(pid,tamanio);//syscalls,base
	recibirDatoDeMSP(sizeof(u_int32_t),(void*)&dir);
	printf("DIRECCION: %x\n",dir);
	return dir;
}

tcb* crearTCB(int modo,u_int32_t dirCod ,u_int32_t dirStack, int tamCod, int pid){
	tcb* hilo = malloc(sizeof(tcb));
	hilo->pid= pid;
	hilo->ppid=-1;
	hilo->tid=numHilos;
	hilo->kernelMode=modo;
	hilo->M = dirCod;
	hilo->P = hilo->M;
	hilo->X= dirStack;
	hilo->S= hilo->X;
	hilo->TamM = tamCod;
	hilo->estaBlock = -1;
	numHilos++;
	return hilo;
}

void crearHiloHijo(tcb* padre,u_int32_t dirStack){
	tcb* hijo = crearTCB(0,padre->M,dirStack,padre->TamM, padre->pid);
	hijo->ppid = padre->tid;
	agregarAReadyOrdenado(hijo);
}

void mandarAPlanificar(tcb*hilo){
	agregarANew(hilo);
}

void*hiloLoader(void *parm){
	thread_parm_t *p = (thread_parm_t *)parm;

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE;		// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(NULL, p->puertoConsola, &hints, &serverInfo); // Notar que le pasamos NULL como IP, ya que le indicamos que use localhost en AI_PASSIVE

	int listenningSocket;
	listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo); // Ya no lo vamos a necesitar

	listen(listenningSocket, BACKLOG);		// IMPORTANTE: listen() es una syscall BLOQUEANTE.
	printf("Escuchando conexiones entrantes.\n");

	fd_set conjunto_temporal;	//Conjunto temporal de lectura que va cambiando
	FD_ZERO(&conjuntoMaestroLoader);	//Limpiar los conjuntos
	FD_ZERO(&conjunto_temporal);

	int fd_maximo,    		//Numero del maximo FD (no de la cantidad de FDs)
	fd_nuevo;      	  //El nuevo FD aceptado

	int tamRecibido,tamCod;
	int i;			          //Iterador

	FD_SET(listenningSocket, &conjuntoMaestroLoader);//Agregar al conjunto el socket de escucha
	fd_maximo = listenningSocket; 				    	//Como tengo uno solo, este es el maximo FD

	struct sockaddr_in info_cliente;		      	// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(info_cliente);

	//Ciclar indefinidamente
	for(;;) {
		conjunto_temporal = conjuntoMaestroLoader; // Copiar el temporal
		if (select(fd_maximo+1, &conjunto_temporal, NULL, NULL, NULL) == -1) {
			perror("select");			//Manejo de errores
			exit(4);
		}
		//Si llega aca, es porque algun FD del "conjunto_temporal" cambio
		//Me fijo cual fue
		for(i = 0; i <= fd_maximo; i++) {
			if (FD_ISSET(i, &conjunto_temporal)) { //Si es el que esta cambiado...
				if (i == listenningSocket) {		//Es el escuchador
					fd_nuevo = accept(listenningSocket, (struct sockaddr *)&info_cliente, &addrlen);	//Acepta la coneccion
					if (fd_nuevo == -1)
						perror("accept");		//Manejo de errores
					else {
						FD_SET(fd_nuevo, &conjuntoMaestroLoader);//Agrega al conjunto maestro este nuvo socket a prestar atencion
						if (fd_nuevo > fd_maximo)			//Si es mas grande que el maximo
							fd_maximo = fd_nuevo;			//Tenes un nuevo maximo
						printf("SERVER: Nueva conexion en socket %d\n", fd_nuevo);
					}
				}else{		//Si el que cambio no fue el escuchador
					if ((tamRecibido = recv(i, &tamCod, sizeof(int), 0)) <= 0) {	//Recive el mensaje
						if (tamRecibido == 0){		//Si no recibio nada y cambio, es porque se cerro
							printf("SERVER: El socket %d colgo\n", i);
							//----CUANDO SE CIERRA UNA CONSOLA----//
							eliminarConsolaDeLaLista(i);
						}else
							perror("recv");	//Manejo de errores
						close(i);						//Cerrar el socket
						FD_CLR(i, &conjuntoMaestroLoader);	//Sacarlo del conjunto maestro
					}else{	//Si efectivamente recibiste algo
						//recibe el tamaño del archivo. Con ese tamaño + el del stack que lo sabe por archivo deberia preguntarle a la MSP si puede reservar.
						//SI PUDO--> SOCKET que espera el codigo, recv que usa el tamaño que le pasaron antes.
						//SI NO PUDO--> AVISA A LA CONSOLA CORRESPONDIENTE "i"

						t_consola* consolaNueva = malloc(sizeof(t_consola));
						consolaNueva->socket = i;
						consolaNueva->pid = contadorDePIDs;
						sem_wait(&mutexListaConsolas);
						list_add(listaConsolas,consolaNueva);
						cantDeConsolas = list_size(listaConsolas);
						sem_post(&mutexListaConsolas);
						printf("\nCantidad de consolas activas: %d\n",cantDeConsolas);
						printf("El tamaño recibido fue: %d\n",tamCod);
						u_int32_t dirCod, dirStk;
						tcb* hilo;
						dirCod = reservarDireccion(tamCod, contadorDePIDs);
						if(dirCod != -1){
							dirStk = reservarDireccion(p->tamanioStack, contadorDePIDs);
							if(dirStk != -1){
								hilo = crearTCB(0,dirCod,dirStk,tamCod,contadorDePIDs);
								mandarAPlanificar(hilo);
								int resp = 1;
								send(i,&resp,sizeof(int),0);
								char codigo[tamCod];
								recibirDatoLoader(i,tamCod,(void*)&codigo);
								printf("\nEl codigo recibido fue: %s\n",codigo);
								//socketMSP para cargar el codigo
								fflush(stdout);
								printf("\nSe va a mandar Cod de TAM: %d",tamCod);
								printf("\nDIRECCION LOGICA DONDE SE ESCRIBE: %x\n",hilo->M);
								escribirEnMemoria(codigo, hilo->pid, hilo->M, tamCod);
								contadorDePIDs++;
								fflush(stdout);
							}else{
								//avisa consola que no pudo reservar
								int respNeg = 0;
								send(i,&respNeg,sizeof(int),0);

							}
						}else{
							//avisa consola que no pudo reservar
							int respNeg = 0;
							send(i,&respNeg,sizeof(int),0);

						}
					}
				}
			}
		}
	}
	pthread_exit(NULL);
}
