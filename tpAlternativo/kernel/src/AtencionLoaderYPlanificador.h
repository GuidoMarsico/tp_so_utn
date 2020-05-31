#include "loaderPlanificador.h"

int esConsola(int numSocket){
	sem_wait(&mutexListaConsolas);
	buscador = numSocket;
	t_consola* consolaAux = list_find(listaConsolas,(void*) _is_consolaBuscada);
	sem_post(&mutexListaConsolas);
	if(consolaAux == NULL){
		return 0;
	}
	return 1;
}

void*hiloAtencionLP(void *parm){
	thread_parm_t *p = (thread_parm_t *)parm;

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(NULL, p->puertoEscucha, &hints, &serverInfo);

	int listenningSocket;
	listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);

	listen(listenningSocket, BACKLOG);
	printf("Escuchando conexiones entrantes.\n");

	fd_set conjunto_temporal;
	FD_ZERO(&conjuntoMaestro);
	FD_ZERO(&conjunto_temporal);

	int fd_maximo,fd_nuevo;

	int tamRecibido,tamCod,quienSoy;
	int i;

	FD_SET(listenningSocket, &conjuntoMaestro);
	fd_maximo = listenningSocket;

	struct sockaddr_in info_cliente;
	socklen_t addrlen = sizeof(info_cliente);

	//Ciclar indefinidamente
	for(;;) {
		conjunto_temporal = conjuntoMaestro;
		if (select(fd_maximo+1, &conjunto_temporal, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(4);
		}
		//Si llega aca, es porque algun FD del "conjunto_temporal" cambio
		//Me fijo cual fue
		for(i = 0; i <= fd_maximo; i++) {
			if (FD_ISSET(i, &conjunto_temporal)) { //Si es el que esta cambiado...
				if (i == listenningSocket) {		//Es el escuchador
					fd_nuevo = accept(listenningSocket, (struct sockaddr *)&info_cliente, &addrlen);	//Acepta la coneccion
					if (fd_nuevo == -1)
						perror("accept");
					else {
						FD_SET(fd_nuevo, &conjuntoMaestro);//Agrega al conjunto maestro este nuvo socket a prestar atencion
						if (fd_nuevo > fd_maximo)			//Si es mas grande que el maximo
							fd_maximo = fd_nuevo;			//Tenes un nuevo maximo
					}
				}else{		//Si el que cambio no fue el escuchador
					if ((tamRecibido = recv(i, &quienSoy, sizeof(int), 0)) <= 0) {	//Recive el mensaje
						if (tamRecibido == 0){		//Si no recibio nada y cambio, es porque se cerro
							//Manejo de error consola eliminarConsolaDeLaLista(i); /cpu manejoErrorPlanificador(i,parm);/ msp
							if(esConsola(i)){
								eliminarConsolaDeLaLista(i);
								desconexion_consola(i);
							}else{
								manejoErrorPlanificador(i,parm);
								desconexion_cpu(i);
							}
						}else
							perror("recv");	//Manejo de errores
						close(i);						//Cerrar el socket
						FD_CLR(i, &conjuntoMaestro);
					}else{
						if(quienSoy == 0){//es la consola
							char* buffer=malloc(5);
							int ok=1;
							send(i,&ok,sizeof(int),0);
							recv(i,buffer,5,MSG_WAITALL);
							if(_esNuevo(buffer)){
								tamCod=0;
								send(i,&tamCod,sizeof(int),0);
								recv(i,&tamCod,sizeof(int),MSG_WAITALL);
								t_consola* consolaNueva = malloc(sizeof(t_consola));
								consolaNueva->socket = i;
								consolaNueva->pid = contadorDePIDs;
								sem_wait(&mutexListaConsolas);
								list_add(listaConsolas,consolaNueva);
								cantDeConsolas = list_size(listaConsolas);
								sem_post(&mutexListaConsolas);
								conexion_consola(i);//Avisa Que se conecto una consola id = socket
								printf("\nCantidad de consolas activas: %d\n",cantDeConsolas);
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
										//socketMSP para cargar el codigo
										fflush(stdout);
										int result = escribirEnMemoria(codigo, hilo->pid, hilo->M, tamCod);
										if (result == -1){
											avisarErrorAConsola(segFaultEscritura,i);
										}
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
							}else if(_esEntradaN(buffer)){
								int k=0;
								int32_t numero;
								send(i,&k,sizeof(int),0);
								recibirDatoLoader(i,sizeof(int32_t),(void*)&numero);
								send(socketCpuEspera,&numero,sizeof(int32_t),0);





							}else if(_esEntradaC(buffer)){

								int num=1;
								int tamCadReal = 0;
								int k=0;
								send(i,&k,sizeof(int),0);
								recv(i,&tamCadReal,sizeof(int),MSG_WAITALL);
								send(i,&num,sizeof(int),0);
								char* cad = malloc(tamCadReal +1);
								recv(i, cad, tamCadReal+1, MSG_WAITALL);
								send(socketCpuEspera,&tamCadReal,sizeof(int),0);
								recv(socketCpuEspera,&num,sizeof(int),MSG_WAITALL);
								send(socketCpuEspera,cad,tamCadReal+1,0);

							}

						}else if (quienSoy == 1) {//es la cpu
							tcb* TCB = malloc(sizeof(tcb));
							char* buffer=malloc(5);
							int ok=1;
							send(i,&ok,sizeof(int),0);
							recv(i,buffer,5,MSG_WAITALL);
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
									conexion_cpu(i);//Avisa que se conecto un cpu con id = socket
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
								BuscadorTCB=TCBRecibido->tid;
								sem_wait(&mutexEXIT);
								if(!list_any_satisfy(fin->elements,(void*) _esTCBBuscado)){
									pasarDeExecAReady(TCBRecibido);
								}
								sem_post(&mutexEXIT);
								//mostrarEstadoColas();
								mostrarColasPanel();
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
									if(esperaHiloORecurso(queue_peek(blockKM))){
										actualizarYBloquear(hilo->tid);
									}else{
										pasarDeExecABlockKM(hilo->tid);
									}
								}else{
									int valorBloq = 0;
									sem_getvalue(&hayAlgoEnBLOQ,&valorBloq);
									if(valorBloq > 0){
										desbloquearTCBPorJoin(hilo->tid);
									}
									eliminarDeMemoria(hilo->pid,hilo->X);
									pasarDeExecAExit(hilo->tid);
									buscarYEliminarHijosDe(hilo->tid);
									buscadorPorPid = hilo->pid;
									t_consola* consolaBuscar = list_find(listaConsolas,(void*) _esConsolaPorPid);
									int socketConsola = consolaBuscar->socket;
									if(hilo->ppid == -1){
										eliminarDeMemoria(hilo->pid,hilo->M);
										avisarErrorAConsola(finalizoHiloPadre,socketConsola);
									}else{
										avisarErrorAConsola(finalizoHiloHijo,socketConsola);
									}
								}
/*crea hilo*/
							}else if(_esCREA(buffer)){
								int ok = 1;
								tcb* padre = queue_pop(blockKM);
								queue_push(blockKM,padre);
								send(i,&ok,sizeof(int),0);
								tcb* tcbRecib = malloc(sizeof(tcb));
								char paquete[sizeof(tcb)];
								t_stream* stream = malloc(sizeof(t_stream));
								recibirDatoPlanificador(i,sizeof(tcb),(void*)&paquete,parm);
								stream->datos=paquete;
								tcbRecib= desserializadorTCB(stream);
								mostrar_InstruccionProg("CREA",tcbRecib);
								padre->A = tcbRecib->A;
								padre->B = tcbRecib->B;
								padre->C = tcbRecib->C;
								padre->D = tcbRecib->D;
								padre->E = tcbRecib->E;
								u_int32_t dirStk = reservarDireccion(p->tamanioStack,padre->pid);
								if(dirStk != -1){
									int ok = 1;
									tcb* hijo = crearTCB(0,padre->M,dirStk,padre->TamM,padre->pid);
									if(padre->ppid == -1){
										hijo->ppid = padre->tid;
									}else{
										hijo->ppid=padre->ppid;
									}
									hijo->P = padre->B;
									hijo->S = hijo->X + (padre->S - padre->X);
									char* contenido = malloc(p->tamanioStack);
									contenido = leerDeMemoria(padre->pid,padre->X,padre->S - padre->X);
									if(contenido != NULL){ //va a ser null cuando el stack del padre no sea escrito, porque cuando
														   //se escribe se le asigna un marco y si no tiene marco cuando lee (o sea
														   //esta en -2) la MSP tira error que no se pudo leer
										int result = escribirEnMemoria(contenido,hijo->pid,hijo->X,padre->S - padre->X);
										if (result==-1){
											buscadorPorPid = padre->pid;
											t_consola* consolaBuscar = list_find(listaConsolas,(void*) _esConsolaPorPid);
											int socketConsola = consolaBuscar->socket;
											avisarErrorAConsola(segFaultEscritura,socketConsola);
											break;
										}
									}
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
								free(stream);
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
								mostrar_InstruccionProg("INTE",TCBRecibido);
								tcb* hilo = buscarTCBen(exec,TCBRecibido->tid,&mutexEXEC,&hayAlgoEnExec);
								hilo->A = TCBRecibido->A;
								hilo->B = TCBRecibido->B;
								hilo->C = TCBRecibido->C;
								hilo->D = TCBRecibido->D;
								hilo->E = TCBRecibido->E;
								hilo->P = TCBRecibido->P;
								hilo->S = TCBRecibido->S;
								hilo->dirSysCall = TCBRecibido->dirSysCall;
								hilo->estaBloq = 0;
								agregarAExec(hilo);
								pasarExecABlock(hilo->tid);
								//mostrarEstadoColas();
								mostrarColasPanel();
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
								//recv(socketConsola,&num,sizeof(int),0);
								socketCpuEspera=i;
								/*int32_t numero;
								recibirDatoLoader(socketConsola,sizeof(int32_t),(void*)&numero);
								send(i,&numero,sizeof(int32_t),0);*/
								buscadorCpu=i;
								sem_wait(&mutexListaCPUs);
								t_cpu* cpuBuscado = list_find(listacpus,(void*)_is_CPUBuscado);
								sem_post(&mutexListaCPUs);
								BuscadorTCB=cpuBuscado->tidactual;
								sem_wait(&mutexEXEC);
								tcb*tcbBuscado=list_find(exec->elements,(void*)_esTCBBuscado);
								sem_post(&mutexEXEC);
								printf(KRED);
								mostrar_InstruccionProg("INNN",tcbBuscado);
								printf(KWHT);
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
								socketCpuEspera=i;

								recibirDatoLoader(socketConsola,sizeof(int),(void*)&num);
								send(socketConsola,&tamCad,sizeof(int32_t),0);
								/*int tamCadReal = 0;
								recv(socketConsola,&tamCadReal,sizeof(int),MSG_WAITALL);
								send(socketConsola,&num,sizeof(int),0);
								char* cad = malloc(tamCadReal +1);
								recv(socketConsola, cad, tamCadReal+1, MSG_WAITALL);
								send(i,&tamCadReal,sizeof(int),0);
								recv(i,&num,sizeof(int),MSG_WAITALL);
								send(i,cad,tamCadReal+1,0);*/
								buscadorCpu=i;
								sem_wait(&mutexListaCPUs);
								t_cpu* cpuBuscado = list_find(listacpus,(void*)_is_CPUBuscado);
								sem_post(&mutexListaCPUs);
								BuscadorTCB=cpuBuscado->tidactual;
								sem_wait(&mutexEXEC);
								tcb*tcbBuscado=list_find(exec->elements,(void*)_esTCBBuscado);
								sem_post(&mutexEXEC);
								printf(KRED);
								mostrar_InstruccionProg("INNC",tcbBuscado);
								printf(KWHT);
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
								buscadorCpu=i;
								sem_wait(&mutexListaCPUs);
								t_cpu* cpuBuscado = list_find(listacpus,(void*)_is_CPUBuscado);
								sem_post(&mutexListaCPUs);
								BuscadorTCB=cpuBuscado->tidactual;
								sem_wait(&mutexEXEC);
								tcb*tcbBuscado=list_find(exec->elements,(void*)_esTCBBuscado);
								sem_post(&mutexEXEC);
								printf(KRED);
								mostrar_InstruccionProg("OUTN",tcbBuscado);
								printf(KWHT);
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
								char* cad = malloc(tamCad+1);
								send(i,&num,sizeof(int),0);
								recv(i,cad,tamCad+1,MSG_WAITALL);
								buscadorPorPid = pid;
								t_consola* consolaBuscar = list_find(listaConsolas,(void*) _esConsolaPorPid);
								int socketConsola = consolaBuscar->socket;
								num = 4;
								send(socketConsola,&num,sizeof(int),0);
								recibirDatoLoader(socketConsola,sizeof(int),(void*)&num);
								send(socketConsola,&tamCad,sizeof(int),0);
								recibirDatoLoader(socketConsola,sizeof(int),(void*)&num);
								send(socketConsola,cad,tamCad+1,0);
								buscadorCpu=i;
								sem_wait(&mutexListaCPUs);
								t_cpu* cpuBuscado = list_find(listacpus,(void*)_is_CPUBuscado);
								sem_post(&mutexListaCPUs);
								BuscadorTCB=cpuBuscado->tidactual;
								sem_wait(&mutexEXEC);
								tcb*tcbBuscado=list_find(exec->elements,(void*)_esTCBBuscado);
								sem_post(&mutexEXEC);
								printf(KRED);
								mostrar_InstruccionProg("OUTC",tcbBuscado);
								printf(KWHT);
								free(cad);
/*join de hilo*/
							}else if(_esJOIN(buffer)){
								bloquear(1,"JOIN"); //lo que hace es ponerle 1 al estaBloq para que despues cuando vuelva el KM lo bloquee
/*bloquea*/
							}else if(_esBLOK(buffer)){
								bloquear(2,"BLOCK"); //lo que hace es ponerle 2 al estaBloq para que despues cuando vuelva el KM lo bloquee
/*despierta*/
							}else if(_esWAKE(buffer)){
								int ok = 1;
								int32_t valorRegB = 0;
								send(i,&ok,sizeof(int),0);
								recv(i,&valorRegB,sizeof(int32_t),MSG_WAITALL);
								desbloquearTCBPorRecurso(valorRegB);

/*dame quantum*/
							}else if(_esDQUA(buffer)){
								send(i, &p->quantum, sizeof(int32_t),0);
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
								recibirDatoPlanificador(i,sizeof(int),(void*)&ok,p);
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
								recibirDatoPlanificador(i,sizeof(int),(void*)&ok,p);
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
									if(tcbEncontrado->kernelMode == 1){
										pasarA(exec,tcbEncontrado,&hayAlgoEnExec,&mutexEXEC);
										cerrarYEliminarTodo(parm);
									}else{
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
								}
/*SEGFAULT ESCRIBIR*/
							}else if(_esSEFE(buffer)){
								buscadorCpu = i;
								t_cpu* cpuABuscar = list_find(listacpus,(void*)_is_CPUBuscado);
								tcb* tcbEncontrado = buscarTCBen(exec,cpuABuscar->tidactual,&mutexEXEC,&hayAlgoEnExec);
								if(tcbEncontrado != NULL){
									if(tcbEncontrado->kernelMode == 1){
										pasarA(exec,tcbEncontrado,&hayAlgoEnExec,&mutexEXEC);
										cerrarYEliminarTodo(parm);
									}else{
										buscadorPorPid = tcbEncontrado->pid;
										t_consola* consolaBuscar = list_find(listaConsolas,(void*) _esConsolaPorPid);
										avisarErrorAConsola(segFaultEscritura,consolaBuscar->socket);
										eliminarDeMemoria(tcbEncontrado->pid,tcbEncontrado->X);
										agregarAExit(tcbEncontrado);
										buscarYEliminarHijosDe(tcbEncontrado->tid);
										if(tcbEncontrado->ppid == -1){
											eliminarDeMemoria(tcbEncontrado->pid,tcbEncontrado->M);
										}
									}
								}
/*SEGFAULT LEER*/
							}else if(_esSEFL(buffer)){
								buscadorCpu = i;
								t_cpu* cpuABuscar = list_find(listacpus,(void*)_is_CPUBuscado);
								tcb* tcbEncontrado = buscarTCBen(exec,cpuABuscar->tidactual,&mutexEXEC,&hayAlgoEnExec);
								if(tcbEncontrado != NULL){
									if(tcbEncontrado->kernelMode == 1){
										pasarA(exec,tcbEncontrado,&hayAlgoEnExec,&mutexEXEC);
										cerrarYEliminarTodo(parm);
									}else{
										buscadorPorPid = tcbEncontrado->pid;
										t_consola* consolaBuscar = list_find(listaConsolas,(void*) _esConsolaPorPid);
										avisarErrorAConsola(segFaultLectura,consolaBuscar->socket);
										eliminarDeMemoria(tcbEncontrado->pid,tcbEncontrado->X);
										agregarAExit(tcbEncontrado);
										buscarYEliminarHijosDe(tcbEncontrado->tid);
										if(tcbEncontrado->ppid == -1){
											eliminarDeMemoria(tcbEncontrado->pid,tcbEncontrado->M);
										}
									}
								}
/*default - Error*/
							}else{
								log_error(logs,"No existe esa primitiva. Abortando...");
								exit(0);
							}
							free(TCB);
							free(buffer);
						}
					}
				}
			}
		}
	}
	pthread_exit(NULL);
}
