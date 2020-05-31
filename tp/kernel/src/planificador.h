#include "loader.h"

void*hiloPlanificador(void *parm){
	thread_parm_t *p2 = (thread_parm_t *)parm;

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE;		// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(NULL, p2->puertoCPU, &hints, &serverInfo); // Notar que le pasamos NULL como IP, ya que le indicamos que use localhost en AI_PASSIVE

	int listenningSocket;
	listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo); // Ya no lo vamos a necesitar

	listen(listenningSocket, BACKLOG);		// IMPORTANTE: listen() es una syscall BLOQUEANTE.
	printf("Escuchando conexiones entrantes.\n");

	fd_set conjunto_temporal;	//Conjunto temporal de lectura que va cambiando
	FD_ZERO(&conjuntoMaestroPlanificador);	//Limpiar los conjuntos
	FD_ZERO(&conjunto_temporal);

	int fd_maximo,    		//Numero del maximo FD (no de la cantidad de FDs)
	fd_nuevo;      	  //El nuevo FD aceptado

	char buffer[5];    	//Buffer
	int longitud_buffer;	//Tamaño del buffer
	int i;			          //Iterador

	FD_SET(listenningSocket, &conjuntoMaestroPlanificador);//Agregar al conjunto el socket de escucha
	fd_maximo = listenningSocket; 				    	//Como tengo uno solo, este es el maximo FD

	struct sockaddr_in info_cliente;		      	// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(info_cliente);

	//Ciclar indefinidamente
	for(;;) {
		conjunto_temporal = conjuntoMaestroPlanificador; // Copiar el temporal
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
						FD_SET(fd_nuevo, &conjuntoMaestroPlanificador);//Agrega al conjunto maestro este nuvo socket a prestar atencion
						if (fd_nuevo > fd_maximo)			//Si es mas grande que el maximo
							fd_maximo = fd_nuevo;			//Tenes un nuevo maximo
						printf("SERVER: Nueva conexion en socket %d\n", fd_nuevo);
					}
				} else {		//Si el que cambio no fue el escuchador
					if ((longitud_buffer = recv(i, buffer, sizeof buffer, 0)) <= 0) {	//Recive el mensaje
						if (longitud_buffer == 0){		//Si no recibio nada y cambio, es porque se cerro
							//------CUANDO SE CANCELA CPU----------------------//
							manejoErrorPlanificador(i,parm);
						}else
							perror("recv");	//Manejo de errores
					} else{		//Si efectivamente recibiste algo
						printf("~~~ Socket #%d mando: \"%.*s\"\n", i, longitud_buffer-1, buffer );
						tcb* TCB = malloc(sizeof(tcb));
/*cpu libre*/
						if(_esLCPU(buffer)){
							sem_wait(&mutexListaCPUs);
							buscadorCpu = i;
							t_cpu* encontrado = list_find(listacpus, (void*)_is_CPUBuscado);
							if(encontrado != NULL){
								encontrado->tidactual = -1;
							}else{
								t_cpu* cpulibre = malloc(sizeof(t_cpu));
								cpulibre->socketCPU=i;
								cpulibre->tidactual = -1;
								list_add(listacpus,cpulibre);
								printf("Se guardo nuevo cpu con num socket :%d\n",cpulibre->socketCPU);
								fflush(stdout);
							}
							sem_post(&hayAlgunCPU);
							sem_post(&mutexListaCPUs);
/*fin quantum*/
						}else if(_esFinQ(buffer)){
							int pedirTCB = 1;
							send(i,&pedirTCB,sizeof(int),0);
							tcb* TCBRecibido = malloc(sizeof(tcb));
							char paquete[sizeof(tcb)];
							t_stream* stream = malloc(sizeof(t_stream));
							recibirDatoPlanificador(i,sizeof(tcb),(void*)&paquete,parm);
							stream->datos=paquete;
							TCBRecibido=desserializadorTCB(stream);
							printf("pid tcb recibido %d modo km %d tid %d\n",TCBRecibido->pid,TCBRecibido->kernelMode,TCBRecibido->tid);
							BuscadorTCB=TCBRecibido->tid;
							sem_wait(&mutexEXIT);
							if(!list_any_satisfy(fin->elements,(void*) _esTCBBuscado)){
								pasarDeExecAReady(TCBRecibido);
							}
							sem_post(&mutexEXIT);
							mostrarEstadoColas();
							fflush(stdout);
/*fin de TCB*/
						}else if(_esXXXX(buffer)){
							int pedirTCB = 1;
							send(i,&pedirTCB,sizeof(int),0);
							tcb* TCBRecibido = malloc(sizeof(tcb));
							char paquete[sizeof(tcb)];
							t_stream* stream = malloc(sizeof(t_stream));
							recibirDatoPlanificador(i,sizeof(tcb),(void*)&paquete,parm);
							stream->datos=paquete;
							TCBRecibido= desserializadorTCB(stream);
							//aca saca de la cola de exec para actualizarlo y lo vuelve a meter para que las funciones
							//siguientes lo puedan encontrar en la cola de ejecucion
							tcb* hilo = buscarTCBen(exec,TCBRecibido->tid,&mutexEXEC,&hayAlgoEnExec);
							hilo->A = TCBRecibido->A;
							hilo->B = TCBRecibido->B;
							hilo->C = TCBRecibido->C;
							hilo->D = TCBRecibido->D;
							hilo->E = TCBRecibido->E;
							agregarAExec(hilo);
							if(hilo->kernelMode == 1){
								pasarDeExecABlockKM(hilo->tid);
							}else{
								eliminarDeMemoria(hilo->pid,hilo->X);
								pasarDeExecAExit(hilo->tid);
								buscarYEliminarHijosDe(hilo->tid);
								if(hilo->ppid == -1){
									eliminarDeMemoria(hilo->pid,hilo->M);
								}
							}
/*crea hilo*/
						}else if(_esCREA(buffer)){
							int ok = 1;
							t_stream* stream = NULL;
							char paquete[sizeof(tcb)];
							tcb* padre = queue_peek(blockKM);
							send(i,&ok,sizeof(int),0);
							recibirDatoPlanificador(i,sizeof(tcb),(void*)&paquete,parm);
							stream->datos = paquete;
							tcb* tcbRecib = desserializadorTCB(stream);
							padre->A = tcbRecib->A;
							padre->B = tcbRecib->B;
							padre->C = tcbRecib->C;
							padre->D = tcbRecib->D;
							padre->E = tcbRecib->E;
							u_int32_t dirStk = reservarDireccion(p2->tamanioStack,padre->pid);
							if(dirStk != -1){
								int ok = 1;
								tcb* hijo = crearTCB(0,padre->M,dirStk,padre->TamM,padre->pid);
								hijo->ppid = padre->tid;
								hijo->P = padre->B;
								char* contenido = malloc(p2->tamanioStack);
								contenido = leerDeMemoria(padre->pid,padre->X,p2->tamanioStack);
								escribirEnMemoria(contenido,hijo->pid,hijo->X,p2->tamanioStack);
								agregarAReadyOrdenado(hijo);
								send(i,&ok,sizeof(int),0);
								recibirDatoPlanificador(i,sizeof(int),(void*)&ok,parm);
								send(i,&hijo->tid,sizeof(int32_t),0);
								free(contenido);
							}else{
								buscadorPorPid = padre->pid;
								t_consola* consolaBuscar = list_find(listaConsolas,(void*) _esConsolaPorPid);
								int socketConsola = consolaBuscar->socket;
								avisarErrorAConsola(creacionStack,socketConsola);
								int error = -1;
								send(i,&error,sizeof(int),0);
								pasarDeExecABlockKM(tcbRecib->tid);
								pasarDeReadyAExit(padre->tid);
							}
							free(tcbRecib);
/*interrupcion*/
						}else if(_esINTE(buffer)){
							int pedirTCB = 1;
							send(i,&pedirTCB,sizeof(int),0);
							tcb* TCBRecibido = malloc(sizeof(tcb));
							char paquete[sizeof(tcb)];
							t_stream* stream = malloc(sizeof(t_stream));
							recibirDatoPlanificador(i,sizeof(tcb),(void*)&paquete,parm);
							stream->datos=paquete;
							TCBRecibido= desserializadorTCB(stream);
							tcb* hilo = buscarTCBen(exec,TCBRecibido->tid,&mutexEXEC,&hayAlgoEnExec);
							hilo->A = TCBRecibido->A;
							hilo->B = TCBRecibido->B;
							hilo->C = TCBRecibido->C;
							hilo->D = TCBRecibido->D;
							hilo->E = TCBRecibido->E;
							hilo->P = TCBRecibido->P;
							hilo->S = TCBRecibido->S;
							hilo->dirSysCall = TCBRecibido->dirSysCall;
							agregarAExec(hilo);
							pasarExecABlock(hilo->tid);
							mostrarEstadoColas();
/*entra numero*/
						}else if(_esINNN(buffer)){
							//recibe pid
							//pide por consola del programa el ingreso de un numero con signo entre -2.147.483.648 y 2.147.483.647
							//le manda el numero al cpu para almacenamiento
							int num = 1;
							send(i,&num,sizeof(int),0);
							int32_t pid;
							recibirDatoPlanificador(i,sizeof(int),(void*)&pid,parm);
							buscadorPorPid = pid;
							t_consola* consolaBuscar = list_find(listaConsolas,(void*) _esConsolaPorPid);
							int socketConsola = consolaBuscar->socket;
							send(socketConsola,&num,sizeof(int),0);
							int32_t numero;
							recibirDatoLoader(socketConsola,sizeof(int32_t),(void*)&numero);
							send(i,&numero,sizeof(int32_t),0);
/*entra cadena*/
						}else if(_esINNC(buffer)){
							//recibe pid, recibe tamaño maximo de cadena
							//pide por consola del programa el ingreso de una cadena de X tamaño
							//le manda la cadena al cpu para almacenamiento
							int num = 1;
							send(i,&num,sizeof(int),0);
							int32_t pid;
							recibirDatoPlanificador(i,sizeof(int32_t),(void*)&pid,parm);
							send(i,&num,sizeof(int),0);
							int32_t tamCad;
							recibirDatoPlanificador(i,sizeof(int32_t),(void*)&tamCad,parm);
							buscadorPorPid = pid;
							t_consola* consolaBuscar = list_find(listaConsolas,(void*) _esConsolaPorPid);
							int socketConsola = consolaBuscar->socket;
							num = 2;
							send(socketConsola,&num,sizeof(int),0);
							recibirDatoLoader(socketConsola,sizeof(int),(void*)&num);
							send(socketConsola,&tamCad,sizeof(int32_t),0);
							int32_t numero;
							recibirDatoLoader(socketConsola,sizeof(int32_t),(void*)&numero);
							send(i,&numero,sizeof(int32_t),0);
/*sale numero*/
						}else if(_esOUTN(buffer)){
							//recibe el numero a mostrar x consola
							//manda a la consola el numero para mostrar
							int num = 1;
							send(i,&num,sizeof(int),0);
							int32_t pid;
							recibirDatoPlanificador(i,sizeof(int32_t),(void*)&pid,parm);
							send(i,&num,sizeof(int),0);
							int numero;
							recibirDatoPlanificador(i,sizeof(int),(void*)&numero,parm);
							buscadorPorPid = pid;
							t_consola* consolaBuscar = list_find(listaConsolas,(void*) _esConsolaPorPid);
							int socketConsola = consolaBuscar->socket;
							num=3;
							send(socketConsola,&num,sizeof(int),0);
							recibirDatoLoader(socketConsola,sizeof(int),(void*)&num);
							send(socketConsola,&numero,sizeof(int),0);
/*sale cadena*/
						}else if(_esOUTC(buffer)){
							//recibe la cadena a mostrar x consola
							//manda a la consola la cadena para mostrar

							int num = 1;
							send(i,&num,sizeof(int),0);
							int32_t pid;
							recibirDatoPlanificador(i,sizeof(int32_t),(void*)&pid,parm);
							send(i,&num,sizeof(int),0);
							int tamCad = 0;
							recibirDatoPlanificador(i,sizeof(int),(void*)&tamCad,parm);
							char* cad = malloc(tamCad);
							send(i,&num,sizeof(int),0);
							recibirDatoPlanificador(i,tamCad,(void*)&cad,parm);
							buscadorPorPid = pid;
							t_consola* consolaBuscar = list_find(listaConsolas,(void*) _esConsolaPorPid);
							int socketConsola = consolaBuscar->socket;
							num = 4;
							send(socketConsola,&num,sizeof(int),0);
							recibirDatoLoader(socketConsola,sizeof(int),(void*)&num);
							send(socketConsola,&tamCad,sizeof(int),0);
							recibirDatoLoader(socketConsola,sizeof(int),(void*)&num);
							send(socketConsola,&cad,tamCad,0);
							free(cad);

/*join de hilo*/
						}else if(_esJOIN(buffer)){
							//TODO: pide el tcbmodificado
							//TODO: bloquea al tcb recibido hasta que el hijo con el tid que diga el registro A termine
/*bloquea*/
						}else if(_esBLOK(buffer)){
							//TODO: pide el tcbmodificado
							//TODO: bloquea al tcb recibido hasta que se libere el recurso apuntado por el registro B
/*despierta*/
						}else if(_esWAKE(buffer)){
							//TODO: desbloquea el primer programa bloqueado por el recurso apuntado por B
							//la evaluacion y decision de si el recurso esta libre o no es hecha por SIGNAL (precompilada)
/*dame quantum*/
						}else if(_esDQUA(buffer)){
							send(i, &p2->quantum, sizeof(int32_t),0);
/*abortar*/
						}else if(_esABOR(buffer)){
							buscadorCpu = i;
							t_cpu* cpuBuscado = list_find(listacpus, (void*) _is_CPUBuscado);
							int32_t TID = cpuBuscado->tidactual;
							tcb* tcbAEliminar = buscarTCBen(exec,TID,&mutexEXEC,&hayAlgoEnExec);
							eliminarDeMemoria(tcbAEliminar->pid,tcbAEliminar->X);
							agregarAExit(tcbAEliminar);
							buscarYEliminarHijosDe(tcbAEliminar->tid);
							if(tcbAEliminar->ppid == -1){
								eliminarDeMemoria(tcbAEliminar->pid,tcbAEliminar->M);
							}
							buscadorPorPid = tcbAEliminar->pid;
							t_consola* consolaBuscar = list_find(listaConsolas,(void*) _esConsolaPorPid);
							int socketConsola = consolaBuscar->socket;
							avisarErrorAConsola(abortoCPU,socketConsola);
/*MALLOC*/
						}else if(_esMALC(buffer)){
							int ok;
							tcb* tcbPadre = malloc(sizeof(tcb));
							tcbPadre = queue_peek(blockKM);
							int32_t pidDelPadre = tcbPadre->pid;
							send(i,&pidDelPadre,sizeof(int32_t),0);
							recibirDatoPlanificador(i,sizeof(int),(void*)&ok,p2);
							if(ok == -1){
								buscadorPorPid = pidDelPadre;
								t_consola* consolaBuscar = list_find(listaConsolas,(void*) _esConsolaPorPid);
								avisarErrorAConsola(memoryOverload,consolaBuscar->socket);
							}
/*FREE*/
						}else if(_esFREE(buffer)){
							int ok;
							tcb* tcbPadre = malloc(sizeof(tcb));
							tcbPadre = queue_peek(blockKM);
							int32_t pidDelPadre = tcbPadre->pid;
							send(i,&pidDelPadre,sizeof(int32_t),0);
							recibirDatoPlanificador(i,sizeof(int),(void*)&ok,p2);
							if(ok == -1){
								buscadorPorPid = pidDelPadre;
								t_consola* consolaBuscar = list_find(listaConsolas,(void*) _esConsolaPorPid);
								avisarErrorAConsola(segFaultEliminar,consolaBuscar->socket);
							}
/*division por 0*/
						}else if(_esDIV0(buffer)){
							buscadorCpu = i;
							t_cpu* cpuABuscar = list_find(listacpus,(void*)_is_CPUBuscado);
							tcb* tcbEncontrado = buscarTCBen(exec,cpuABuscar->tidactual,&mutexEXEC,&hayAlgoEnExec);
							if(tcbEncontrado != NULL){
								buscadorPorPid = tcbEncontrado->pid;
								t_consola* consolaBuscar = list_find(listaConsolas,(void*) _esConsolaPorPid);
								avisarErrorAConsola(divisionPorCero,consolaBuscar->socket);
								eliminarDeMemoria(tcbEncontrado->pid,tcbEncontrado->X);
								agregarAExit(tcbEncontrado);
								buscarYEliminarHijosDe(tcbEncontrado->tid);
								if(tcbEncontrado->ppid == -1){
									eliminarDeMemoria(tcbEncontrado->pid,tcbEncontrado->M);
								}
							}
/*default - Error*/
						}else{
							log_error(logs,"No existe esa primitiva. Abortando...");
						}
						free(TCB);
					}
				}
			}
		}
	}
	pthread_exit(NULL);
}
