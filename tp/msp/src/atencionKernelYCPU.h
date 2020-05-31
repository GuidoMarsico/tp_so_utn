#include "hiloConsola.h"

int primero = 1;

void atencionKernelYCPU(void* parm){
	thread_parm_t *p2 = (thread_parm_t *)parm;
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE;		// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(NULL, p2->puertoEscucha, &hints, &serverInfo); // Notar que le pasamos NULL como IP, ya que le indicamos que use localhost en AI_PASSIVE

	/* Necesitamos un socket que escuche las conecciones entrantes */
	int listenningSocket;
	listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo); // Ya no lo vamos a necesitar

	listen(listenningSocket, BACKLOG);		// IMPORTANTE: listen() es una syscall BLOQUEANTE.
	printf("Escuchando conexiones entrantes.\n");

	fd_set	conjunto_maestro,   //Conjunto maestro para comprar
	conjunto_temporal;	//Conjunto temporal de lectura que va cambiando
	FD_ZERO(&conjunto_maestro);	//Limpiar los conjuntos
	FD_ZERO(&conjunto_temporal);

	int fd_maximo,    		//Numero del maximo FD (no de la cantidad de FDs)
	fd_nuevo;      	  //El nuevo FD aceptado

	int tamRecibido,opcion;
	int i;			          //Iterador

	FD_SET(listenningSocket, &conjunto_maestro);//Agregar al conjunto el socket de escucha
	fd_maximo = listenningSocket; 				    	//Como tengo uno solo, este es el maximo FD

	struct sockaddr_in info_cliente;		      	// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(info_cliente);

	//Ciclar indefinidamente
	for(;;) {
		if(malEstado){
			eliminarTodoLoSwappeado();
			break;
		}
		conjunto_temporal = conjunto_maestro; // Copiar el temporal
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
						FD_SET(fd_nuevo, &conjunto_maestro);//Agrega al conjunto maestro este nuvo socket a prestar atencion
						if (fd_nuevo > fd_maximo)			//Si es mas grande que el maximo
							fd_maximo = fd_nuevo;			//Tenes un nuevo maximo
						if(!primero) log_info(logs,"Se conectó un CPU");
					}
				} else {		//Si el que cambio no fue el escuchador
					if ((tamRecibido = recv(i, &opcion, sizeof(int), MSG_WAITALL)) <= 0) {//Recive el mensaje
						if (tamRecibido == 0){		//Si no recibio nada y cambio, es porque se cerro
							manejoError(i,&conjunto_maestro);
						}else
							perror("recv");	//Manejo de errores
					}else{ //Si efectivamente recibiste algo

						if (opcion == 0){
							kernelSocket = i;
							log_info(logs,"Se conectó el Kernel");
							primero = 0;

						}else if (opcion == 1){// OPCION RESERVAR (1)
							if(i == kernelSocket){
								printf(KGRN);
								log_info(logs,"El Kernel solicito RESERVAR");
								printf(KWHT);
							}else{
								printf(KGRN);
								log_info(logs,"Una CPU solicito RESERVAR");
								printf(KWHT);
							}
							t_stream* stream = malloc (sizeof (stream));
							t_reservar_destruir* rd = malloc (sizeof (t_reservar_destruir));
							char rdRecibido[sizeof(t_reservar_destruir)];
							fflush(stdout);
							recv(i, (void*)&rdRecibido, sizeof(t_reservar_destruir), MSG_WAITALL);
							stream->dataARecibir = rdRecibido;
							rd = reservar_destruir_DESserializacion(stream);
							log_info(logs,"Parametros:");
							log_info(logs,"PID: %d",rd->pid);
							log_info(logs,"Tamaño a reservar: %d",rd->tam_o_base);
							u_int32_t direccion;
							direccion = crearSegmento(rd->pid,rd->tam_o_base);
							if(direccion == -1){
								log_error(logs,"No se pudo reservar memoria");
							}else{
								log_info(logs,"El pedido de creacion de segmento se pudo satisfacer correctamente");
							}
							send(i,&direccion,sizeof(u_int32_t),0);
							free(stream);
							free(rd);

						}else if (opcion == 2){//OPCION DESTRUIR (2)
							if(i == kernelSocket){
								printf(KRED);
								log_info(logs,"El Kernel solicito DESTRUIR");
								printf(KWHT);
							}else{
								printf(KRED);
								log_info(logs,"Una CPU solicito DESTRUIR");
								printf(KWHT);
							}
							t_stream* stream = malloc (sizeof (stream));
							t_reservar_destruir* rd = malloc (sizeof (t_reservar_destruir));
							char rdRecibido[sizeof(t_reservar_destruir)];
							fflush(stdout);
							recv(i, (void*)&rdRecibido, sizeof(t_reservar_destruir), MSG_WAITALL);
							stream->dataARecibir = rdRecibido;
							rd = reservar_destruir_DESserializacion(stream);
							log_info(logs,"Parametros:");
							log_info(logs,"PID: %d",rd->pid);
							log_info(logs,"BASE: %d",rd->tam_o_base);
							int respuesta;
							int error = -1;
							respuesta = destruirSegmento(rd->pid, rd->tam_o_base);
							if(respuesta == -1){
								send(i,&error, sizeof(int),0);
								log_error(logs,"No se pudo destruir segmento");
							}else{
								send(i,&respuesta, sizeof(int),0);
								log_info(logs,"El pedido de destruccion de segmento se pudo satisfacer correctamente");
							}
							free(stream);
							free(rd);

						}else if (opcion == 3){//OPCION LEER (3)
							if(i == kernelSocket){
								printf(KBLU);
								log_info(logs,"El Kernel solicito LEER");
								printf(KWHT);
							}else{
								printf(KBLU);
								log_info(logs,"Una CPU solicito LEER");
								printf(KWHT);
							}
							t_stream* stream = malloc (sizeof (stream));
							t_leer* l = malloc (sizeof (t_leer));
							char lRecibido[sizeof(t_leer)];
							fflush(stdout);
							recv(i, (void*)&lRecibido, sizeof(t_leer), MSG_WAITALL);
							stream->dataARecibir = lRecibido;
							l = leer_DESserializacion(stream);
							log_info(logs, "Parametros:");
							log_info(logs,"PID: %d",l->pid);
							log_info(logs,"Dir lógica: %d",l->dirLogica);
							log_info(logs,"Tam a leer: %d",l->tam);
							int error = -1;
							char* varContenidoKernel = malloc(l->tam+1);
							int respuesta;
							respuesta = solicitarMemoria(l->pid, l->dirLogica, l->tam);
							if(respuesta == -1){
								send(i,&error, sizeof(int),0);
							}else{
								send(i,&respuesta, sizeof(int),0);
								recibirDato(i,sizeof(int),(void*)&respuesta,&conjunto_maestro);
								memcpy(varContenidoKernel,memoriaPrincipal + respuesta, l->tam);
								varContenidoKernel[l->tam] = '\0';
								send(i, varContenidoKernel, l->tam+1, 0);
							}
							free(varContenidoKernel);
							free(stream);
							free(l);

						}else if (opcion == 4){//OPCION ESCRIBIR (4)
							if(i == kernelSocket){
								printf(KYEL);
								log_info(logs,"El Kernel solicito ESCRIBIR");
								printf(KWHT);
							}else{
								printf(KYEL);
								log_info(logs,"Una CPU solicito ESCRIBIR");
								printf(KWHT);
							}
							t_stream* stream = malloc (sizeof (stream));
							t_escribir* e = malloc (sizeof (t_escribir));
							int tamArecibir = 0;
							recv(i, &tamArecibir, sizeof(int), MSG_WAITALL);
							char eRecibido[tamArecibir];
							int ok = 1;
							send(i,&ok,sizeof(int),0);
							recv(i, (void*)&eRecibido, tamArecibir, MSG_WAITALL);
							stream->dataARecibir = eRecibido;
							e = escribir_DESserializacion(stream,tamArecibir);
							log_info(logs, "Parametros:");
							log_info(logs,"PID: %d",e->pid);
							log_info(logs,"Dir lógica: %d",e->dirLogica);
							log_info(logs,"Tam a escribir: %d",e->tam);
							log_info(logs,"Bytes a ESCRIBIR:");
							imprimirCodigoCompleto(e->tam,e->bytesAEscribir);
							int respuesta;
							respuesta = escribirMemoria(e->pid, e->dirLogica, e->bytesAEscribir, e->tam);
							if(respuesta == -1){
								send(i,&respuesta, sizeof(int),0);
								printf("\nNo se pudo escribir en memoria\n");
							}else{
								send(i,&ok, sizeof(int),0);
							}
							free(stream);
							free(e);
						}
					}
				}
			}
		}
	}
	exit(0);
}
