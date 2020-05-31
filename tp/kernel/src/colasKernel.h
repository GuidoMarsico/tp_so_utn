#include <commons/collections/queue.h>
#include <semaphore.h>
#include "serializacionKernel.h"

#define KGRN  "\x1B[32m"
#define KNRM  "\x1B[0m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

t_queue* nuevo;
t_queue* ready;
t_queue* exec;
t_queue* block;
t_queue* blockKM;
t_queue* fin;
t_list* listacpus;
t_list* listaConsolas;

sem_t mutexNEW;
sem_t mutexREADY;
sem_t mutexEXEC;
sem_t mutexEXIT;
sem_t hayAlgoEnNew;
sem_t hayAlgoEnReady;
sem_t hayAlgoEnExec;
sem_t hayAlgoEnExit;
sem_t mutexBLOQ;
sem_t hayAlgoEnBLOQ;
sem_t hayAlgoEnBlockKM;
sem_t mutexBlockKM;
sem_t mutexListaCPUs;
sem_t hayAlgunCPU;
sem_t mutexListaConsolas;

typedef struct{
	int32_t pid;
	int32_t ppid;
	int32_t tid;
	int32_t kernelMode;
	u_int32_t M;
	int TamM;
	u_int32_t P;
	u_int32_t X;
	u_int32_t S;
	int32_t A;
	int32_t B;
	int32_t C;
	int32_t D;
	int32_t E;
	u_int32_t dirSysCall;
	int estaBlock;
} __attribute__ ((__packed__)) tcb;

typedef struct{
	int tam;
	char* datos;
}t_stream;

t_stream* serializadorTcb(tcb* hilo){
	char* data= malloc(sizeof(int32_t)*10 + sizeof(u_int32_t)*5 + sizeof(int) * 2);
	t_stream* stream = malloc(sizeof(t_stream));
	int offset=0, tmp_size=0;
	memcpy(data,&hilo->A,tmp_size =sizeof(int32_t));
	offset=tmp_size;
	memcpy(data+offset,&hilo->B,tmp_size =sizeof(int32_t));
	offset+=tmp_size;
	memcpy(data+offset,&hilo->C,tmp_size =sizeof(int32_t));
	offset+=tmp_size;
	memcpy(data+offset,&hilo->D,tmp_size =sizeof(int32_t));
	offset+=tmp_size;
	memcpy(data+offset,&hilo->E,tmp_size =sizeof(int32_t));
	offset+=tmp_size;
	memcpy(data+offset,&hilo->kernelMode,tmp_size =sizeof(int32_t));
	offset+=tmp_size;
	memcpy(data+offset,&hilo->pid,tmp_size =sizeof(int32_t));
	offset+=tmp_size;
	memcpy(data+offset+offset,&hilo->ppid,tmp_size =sizeof(int32_t));
	offset+=tmp_size;;
	memcpy(data+offset,&hilo->tid,tmp_size =sizeof(int32_t));
	offset+=tmp_size;
	memcpy(data+offset,&hilo->M,tmp_size =sizeof(u_int32_t));
	offset+=tmp_size;
	memcpy(data+offset,&hilo->P,tmp_size =sizeof(u_int32_t));
	offset+=tmp_size;
	memcpy(data+offset,&hilo->S,tmp_size =sizeof(u_int32_t));
	offset+=tmp_size;
	memcpy(data+offset,&hilo->X,tmp_size =sizeof(u_int32_t));
	offset+=tmp_size;
	memcpy(data+offset,&hilo->dirSysCall,tmp_size =sizeof(u_int32_t));
	offset+=tmp_size;
	memcpy(data+offset,&hilo->TamM,tmp_size =sizeof(int));
	offset+=tmp_size;
	memcpy(data+offset,&hilo->estaBlock,tmp_size =sizeof(int));
	stream->tam = offset + tmp_size;
	stream->datos=data;
	return stream;
}

tcb* desserializadorTCB(t_stream* stream){
	tcb* hilo = malloc(sizeof(tcb));
	int offset=0, tmp_size=0;
	memcpy(&hilo->A,stream->datos,tmp_size =sizeof(int32_t));
	offset=tmp_size;
	memcpy(&hilo->B,stream->datos+offset,tmp_size =sizeof(int32_t));
	offset+=tmp_size;
	memcpy(&hilo->C,stream->datos+offset,tmp_size =sizeof(int32_t));
	offset+=tmp_size;
	memcpy(&hilo->D,stream->datos+offset,tmp_size =sizeof(int32_t));
	offset+=tmp_size;
	memcpy(&hilo->E,stream->datos+offset,tmp_size =sizeof(int32_t));
	offset+=tmp_size;
	memcpy(&hilo->kernelMode,stream->datos+offset,tmp_size =sizeof(int32_t));
	offset+=tmp_size;
	memcpy(&hilo->pid,stream->datos+offset,tmp_size =sizeof(int32_t));
	offset+=tmp_size;
	memcpy(&hilo->ppid,stream->datos+offset,tmp_size =sizeof(int32_t));
	offset+=tmp_size;;
	memcpy(&hilo->tid,stream->datos+offset,tmp_size =sizeof(int32_t));
	offset+=tmp_size;
	memcpy(&hilo->M,stream->datos+offset,tmp_size =sizeof(u_int32_t));
	offset+=tmp_size;
	memcpy(&hilo->P,stream->datos+offset,tmp_size =sizeof(u_int32_t));
	offset+=tmp_size;
	memcpy(&hilo->S,stream->datos+offset,tmp_size =sizeof(u_int32_t));
	offset+=tmp_size;
	memcpy(&hilo->X,stream->datos+offset,tmp_size =sizeof(u_int32_t));
	offset+=tmp_size;
	memcpy(&hilo->dirSysCall,stream->datos+offset,tmp_size =sizeof(u_int32_t));
	offset+=tmp_size;
	memcpy(&hilo->TamM,stream->datos+offset,tmp_size =sizeof(int));
	offset+=tmp_size;
	memcpy(&hilo->estaBlock,stream->datos+offset,tmp_size =sizeof(int));
	return hilo;
}

int _esTCBBuscado(tcb* elem){
	return elem->tid == BuscadorTCB;
}

void crearColas (){
	nuevo = queue_create();
	ready= queue_create();
	exec= queue_create();
	block = queue_create();
	blockKM = queue_create();
	fin = queue_create();
	listacpus = list_create();
	listaConsolas = list_create();
	contadorDePIDs = 1;
	cantDeConsolas = 0;
}

void pasarA(t_queue* cola , tcb* hilo,sem_t* hayAlgo,sem_t* mutex){
	sem_wait(mutex);
	queue_push(cola,hilo);
	sem_post(mutex);
	sem_post(hayAlgo);
}

void agregarABlockKernelMode(tcb*hilo){
	pasarA(blockKM,hilo,&hayAlgoEnBlockKM,&mutexBlockKM);
}

void agregarAExit(tcb*hilo){
	pasarA(fin,hilo,&hayAlgoEnExit,&mutexEXIT);
}

void agregarAExec(tcb*hilo){
	pasarA(exec,hilo,&hayAlgoEnExec,&mutexEXEC);
}

void agregarABlock(tcb*hilo){
	pasarA(block,hilo,&hayAlgoEnBLOQ,&mutexBLOQ);
}

void agregarANew(tcb* hilo){
	pasarA(nuevo,hilo,&hayAlgoEnNew,&mutexNEW);
}

bool compararPrioridades (const void* elem1,const void* elem2){
	tcb* x;
	tcb* y;
	x = (tcb*) elem1;
	y = (tcb*) elem2;
	return (x->kernelMode >= y->kernelMode);
}

void agregarAReadyOrdenado(tcb* hilo){
	sem_wait(&mutexREADY);
	queue_push(ready,hilo);
	bool(*comparador)(void*,void*) = compararPrioridades;
	list_sort(ready->elements,comparador);
	sem_post(&mutexREADY);
	sem_post(&hayAlgoEnReady);
}

tcb* buscarTCBen(t_queue* cola, int32_t tid, sem_t* mutex, sem_t* hayAlgo){
	sem_wait(hayAlgo);
	sem_wait(mutex);
	t_queue* colaAux = queue_create();
	tcb* hilo = queue_pop(cola);
	while(hilo->tid != tid){
		queue_push(colaAux, hilo);
		hilo = queue_pop(cola);
	}
	while(!queue_is_empty(colaAux)){
		queue_push(cola, queue_pop(colaAux));
	}
	queue_destroy(colaAux);
	sem_post(mutex);
	return hilo;
}

int esperaKM(tcb* elem){
	return elem->estaBlock == 0;
}

tcb* pasarDeBlockABlockKM(){
	sem_wait(&hayAlgoEnBLOQ);
	sem_wait(&mutexBLOQ);
	tcb* tcbABloq;
	buscar:
	tcbABloq = list_remove_by_condition(block->elements,(void*)esperaKM);
	if(tcbABloq == NULL) goto buscar;
	agregarABlockKernelMode(tcbABloq);
	sem_post(&mutexBLOQ);
	return tcbABloq;
}

void pasarDeNewAReady(){
	sem_wait(&hayAlgoEnNew);
	sem_wait(&mutexNEW);
	agregarAReadyOrdenado(queue_pop(nuevo));
	sem_post(&mutexNEW);
}

void pasarDeReadyAExec(){
	sem_wait(&hayAlgoEnReady);
	sem_wait(&mutexREADY);
	agregarAExec(queue_pop(ready));
	sem_post(&mutexREADY);
}

void pasarDeExecAReady(tcb* tcbModif){
	tcb* TCB = buscarTCBen(exec,tcbModif->tid,&mutexEXEC,&hayAlgoEnExec);
	TCB->A = tcbModif->A;
	TCB->B = tcbModif->B;
	TCB->C = tcbModif->C;
	TCB->D = tcbModif->D;
	TCB->E = tcbModif->E;
	TCB->P = tcbModif->P;
	TCB->S = tcbModif->S;
	agregarAReadyOrdenado(TCB);
}

void pasarExecABlock(int32_t tid){
	agregarABlock(buscarTCBen(exec,tid,&mutexEXEC,&hayAlgoEnExec));
}

void pasarDeExecAExit(int32_t tid){
	agregarAExit(buscarTCBen(exec,tid,&mutexEXEC,&hayAlgoEnExec));
}

//Cuando termina la exec de un hilo (avisado por CPU), y el hilo es el de KM
//se llama a esta funcion para copiarle los datos al hilo que esta en la cola de
//KM, pasarlo a la de READY.. Y el hilo KM a la cola KM
void pasarDeExecABlockKM(int32_t tid){
	tcb* hilo2;
	hilo2 = buscarTCBen(exec,tid,&mutexEXEC,&hayAlgoEnExec);
	sem_wait(&hayAlgoEnBlockKM);
	sem_wait(&mutexBlockKM);
	tcb* hilo;
	hilo = queue_pop(blockKM);
	sem_post(&mutexBlockKM);
	hilo->A = hilo2->A;
	hilo->B = hilo2->B;
	hilo->C = hilo2->C;
	hilo->D = hilo2->D;
	hilo->E = hilo2->E;
	hilo2->pid = 0;
	agregarAReadyOrdenado(hilo);
	agregarABlockKernelMode(hilo2);
}

//Se fija la cant de hilos en BLOCK, si hay alguno y en la BLOCKKM esta el hilo
// de KM entonces hace el cambio, pasando el hilo KM a ready y el hilo de bloqueado
// queda en la cola de BLOCKKM (previo copiar todos los campos)
void pasarDeBlockKMAReady(){
	tcb* hilo;
	int valor;
	empezar:
	sem_getvalue(&hayAlgoEnBLOQ,&valor);
	tcb* hiloKernel;
	hiloKernel = queue_peek(blockKM);
	if(valor > 0 && hiloKernel->kernelMode == 1){
		sem_wait(&mutexBlockKM);
		sem_wait(&hayAlgoEnBlockKM);
		hilo = queue_pop(blockKM);
		sem_post(&mutexBlockKM);
		tcb* hilo2;
		hilo2 = pasarDeBlockABlockKM();
		hilo->A = hilo2->A;
		hilo->B = hilo2->B;
		hilo->C = hilo2->C;
		hilo->D = hilo2->D;
		hilo->E = hilo2->E;
		hilo->P = hilo2->dirSysCall;
		hilo->pid = hilo2->pid;
		agregarAReadyOrdenado(hilo);
	}
	goto empezar;
}

//pasar ready a exit , ejemplo cuando muere el padre de un hilo , con el hijo en ready
void pasarDeReadyAExit(int32_t tid){
	agregarAExit(buscarTCBen(ready,tid,&mutexREADY,&hayAlgoEnReady));
}

//pasar block a exit, mismo ejemplo la muerte de un hilo padre
void pasarDeBlockAExit(int32_t tid){
	agregarAExit(buscarTCBen(block,tid,&mutexBLOQ,&hayAlgoEnBLOQ));
}

void mostrarEstadoDe(t_queue* cola){
	t_queue* colaAux;
	colaAux = queue_create();
	tcb* hilo;
	while(!queue_is_empty(cola)){
		hilo= queue_pop(cola);
		queue_push(colaAux, hilo);
		printf("Hilo_ID: %d\t",hilo->tid);
	}
	while(!queue_is_empty(colaAux)){
		queue_push(cola, queue_pop(colaAux));
	}
	queue_destroy(colaAux);
}

void mostrarEstadoColas(){
	printf(KYEL "\n------El estado de las colas es--------\n");
	printf("El estado de la COLA NEW: ");
	sem_wait(&mutexNEW);
	mostrarEstadoDe(nuevo);
	sem_post(&mutexNEW);
	printf("\n");
	printf("El estado de la COLA READY: ");
	sem_wait(&mutexREADY);
	mostrarEstadoDe(ready);
	sem_post(&mutexREADY);
	printf("\n");
	printf("El estado de la COLA EJECUCION: ");
	sem_wait(&mutexEXEC);
	mostrarEstadoDe(exec);
	sem_post(&mutexEXEC);
	printf("\n");
	printf("El estado de la COLA BLOQ: ");
	sem_wait(&mutexBLOQ);
	mostrarEstadoDe(block);
	sem_post(&mutexBLOQ);
	printf("\n");
	printf("El estado de la COLA BLOCKKM: ");
	sem_wait(&mutexBlockKM);
	mostrarEstadoDe(blockKM);
	sem_post(&mutexBlockKM);
	printf("\n");
	printf("El estado de la COLA EXIT: ");
	sem_wait(&mutexEXIT);
	mostrarEstadoDe(fin);
	sem_post(&mutexEXIT);
	printf("\n" KNRM);
}

tcb* buscarConPidElTcbEn(t_queue* cola, int32_t pid, sem_t* mutex, sem_t* hayAlgo){
	sem_wait(hayAlgo);
	sem_wait(mutex);
	t_queue* colaAux = queue_create();
	tcb* hilo = queue_pop(cola);
	while(hilo->pid != pid && !queue_is_empty(cola)){
		queue_push(colaAux, hilo);
		hilo = queue_pop(cola);
	}
	while(!queue_is_empty(colaAux)){
		queue_push(cola, queue_pop(colaAux));
	}
	queue_destroy(colaAux);
	sem_post(mutex);
	return hilo;
}

int eliminarDeMemoria(int32_t pid, u_int32_t base){
	int opcion = 2;
	send(socketMSP, &opcion, sizeof(int), 0);
	reservar_destruir_enviar(pid,base);//syscalls,base
	int rta = 0;
	recv(socketMSP,&rta,sizeof(int),MSG_WAITALL);
	if(rta == -1){
		//TODO: avisar a la consola que no se pudo eliminar
		printf("\nNo se pudo eliminar memoria\n");
		return -1;
	}
	return 0;
}

tcb* buscarHijoEn(t_queue* cola, int32_t tid, sem_t* mutex, sem_t* hayAlgo){
	sem_wait(hayAlgo);
	sem_wait(mutex);
	t_queue* colaAux = queue_create();
	tcb* hilo = queue_pop(cola);
	while(hilo->ppid != tid && !queue_is_empty(cola)){
		queue_push(colaAux, hilo);
		hilo = queue_pop(cola);
	}
	while(!queue_is_empty(colaAux)){
		queue_push(cola, queue_pop(colaAux));
	}
	queue_destroy(colaAux);
	sem_post(mutex);
	return hilo;
}

void buscarYEliminarHijosDe(int32_t tid){
	tcb* tcbDeEliminacion;
	int valorNew;
	int valorReady;
	int valorBlock;
	int valorExec;

	//se busca y elimina de la cola de new
	saltoNew:
	sem_getvalue(&hayAlgoEnNew,&valorNew);
	if(valorNew > 0){
		tcbDeEliminacion = buscarHijoEn(nuevo, tid, &mutexNEW, &hayAlgoEnNew);
		if(tcbDeEliminacion->ppid == tid){
			eliminarDeMemoria(tcbDeEliminacion->pid,tcbDeEliminacion->X);
			agregarAExit(tcbDeEliminacion);
			goto saltoNew;
		}
		agregarANew(tcbDeEliminacion);
	}

	//se busca y elimina de la cola de ready
	saltoReady:
	sem_getvalue(&hayAlgoEnReady,&valorReady);
	if(valorReady > 0){
		tcbDeEliminacion = buscarHijoEn(ready, tid, &mutexREADY, &hayAlgoEnReady);
		if(tcbDeEliminacion->ppid == tid){
			eliminarDeMemoria(tcbDeEliminacion->pid,tcbDeEliminacion->X);
			agregarAExit(tcbDeEliminacion);
			goto saltoReady;
		}
		agregarAReadyOrdenado(tcbDeEliminacion);
	}

	//se busca y elimina de la cola de exec
	saltoExec:
	sem_getvalue(&hayAlgoEnExec,&valorExec);
	if(valorExec > 0){
		tcbDeEliminacion = buscarHijoEn(exec, tid, &mutexEXEC, &hayAlgoEnExec);
		while(tcbDeEliminacion->ppid == tid){
			eliminarDeMemoria(tcbDeEliminacion->pid,tcbDeEliminacion->X);
			agregarAExit(tcbDeEliminacion);
			goto saltoExec;
		}
		agregarAExec(tcbDeEliminacion);
	}

	//se busca y elimina de la cola de block
	saltoBlock:
	sem_getvalue(&hayAlgoEnBLOQ,&valorBlock);
	if(valorBlock >0){
		tcbDeEliminacion = buscarHijoEn(block, tid, &mutexBLOQ, &hayAlgoEnBLOQ);
		while(tcbDeEliminacion->ppid == tid){
			eliminarDeMemoria(tcbDeEliminacion->pid,tcbDeEliminacion->X);
			agregarAExit(tcbDeEliminacion);
			goto saltoBlock;
		}
		agregarABlock(tcbDeEliminacion);
	}
	mostrarEstadoColas();
}

void buscaYEliminaEnTodosLadosSegun(int32_t pidDeConsola){
	//encuentra el TCB a eliminar con el pidDeConsola, consigue el TID
	//se busca el tcb que coincida con el que haya q eliminar (en todas las colas y tantas veces como sea necesario)
	//se elimina el segmento de stk de ese tcb
	tcb* tcbDeEliminacion;
	int valorNew;
	int valorReady;
	int valorBlock;
	int valorExec;

	//se busca y elimina de la cola de new
	saltoNew:
	sem_getvalue(&hayAlgoEnNew,&valorNew);
	if(valorNew > 0){
		tcbDeEliminacion = buscarConPidElTcbEn(nuevo, pidDeConsola, &mutexNEW, &hayAlgoEnNew);
		if(tcbDeEliminacion->pid == pidDeConsola){
			eliminarDeMemoria(pidDeConsola,tcbDeEliminacion->X);
			buscarYEliminarHijosDe(tcbDeEliminacion->tid);
			if(tcbDeEliminacion->ppid == -1){
				eliminarDeMemoria(tcbDeEliminacion->pid,tcbDeEliminacion->M);
			}
			agregarAExit(tcbDeEliminacion);
			goto saltoNew;
		}
		agregarANew(tcbDeEliminacion);
	}

	//se busca y elimina de la cola de ready
	saltoReady:
	sem_getvalue(&hayAlgoEnReady,&valorReady);
	if(valorReady > 0){
		tcbDeEliminacion = buscarConPidElTcbEn(ready, pidDeConsola, &mutexREADY, &hayAlgoEnReady);
		if(tcbDeEliminacion->pid == pidDeConsola){
			eliminarDeMemoria(pidDeConsola,tcbDeEliminacion->X);
			buscarYEliminarHijosDe(tcbDeEliminacion->tid);
			if(tcbDeEliminacion->ppid == -1){
				eliminarDeMemoria(tcbDeEliminacion->pid,tcbDeEliminacion->M);
			}
			agregarAExit(tcbDeEliminacion);
			goto saltoReady;
		}
		agregarAReadyOrdenado(tcbDeEliminacion);
	}

	//se busca y elimina de la cola de exec
	saltoExec:
	sem_getvalue(&hayAlgoEnExec,&valorExec);
	if(valorExec > 0){
		tcbDeEliminacion = buscarConPidElTcbEn(exec, pidDeConsola, &mutexEXEC, &hayAlgoEnExec);
		if(tcbDeEliminacion->pid == pidDeConsola){
			eliminarDeMemoria(pidDeConsola,tcbDeEliminacion->X);
			buscarYEliminarHijosDe(tcbDeEliminacion->tid);
			if(tcbDeEliminacion->ppid == -1){
				eliminarDeMemoria(tcbDeEliminacion->pid,tcbDeEliminacion->M);
			}
			agregarAExit(tcbDeEliminacion);
			goto saltoExec;
		}
		agregarAExec(tcbDeEliminacion);
	}

	//se busca y elimina de la cola de block
	saltoBlock:
	sem_getvalue(&hayAlgoEnBLOQ,&valorBlock);
	if(valorBlock >0){
		tcbDeEliminacion = buscarConPidElTcbEn(block, pidDeConsola, &mutexBLOQ, &hayAlgoEnBLOQ);
		if(tcbDeEliminacion->pid == pidDeConsola){
			eliminarDeMemoria(pidDeConsola,tcbDeEliminacion->X);
			buscarYEliminarHijosDe(tcbDeEliminacion->tid);
			if(tcbDeEliminacion->ppid == -1){
				eliminarDeMemoria(tcbDeEliminacion->pid,tcbDeEliminacion->M);
			}
			agregarAExit(tcbDeEliminacion);
			goto saltoBlock;
		}
		agregarABlock(tcbDeEliminacion);
	}
	mostrarEstadoColas();
}

void eliminarConsolaDeLaLista(int socketConsola){
	sem_wait(&mutexListaConsolas);
	int pidDeConsola;
	buscador = socketConsola;
	t_consola* consolaAux = list_find(listaConsolas,(void*) _is_consolaBuscada);
	pidDeConsola = consolaAux->pid;
	//FIXME: QUE PASA? log_info(logs,"La consola se eliminará");
	printf("\nPID: %d",pidDeConsola);
	list_remove_and_destroy_by_condition(listaConsolas,(void*) _is_consolaBuscada,(void*) _consolaDestroy);
	cantDeConsolas = list_size(listaConsolas);
	printf("\nCantidad de consolas activas: %d\n",cantDeConsolas);
	fflush(stdout);
	buscaYEliminaEnTodosLadosSegun(pidDeConsola); //FIXME: probar que funcione bien
	sem_post(&mutexListaConsolas);
}

void finalizarEjecucion(thread_parm_t* parm){
	sem_destroy(&hayAlgoEnNew);
	sem_destroy(&hayAlgoEnBlockKM);
	sem_destroy(&hayAlgoEnExec);
	sem_destroy(&hayAlgoEnExit);
	sem_destroy(&hayAlgoEnReady);
	sem_destroy(&mutexBLOQ);
	sem_destroy(&hayAlgoEnBLOQ);
	sem_destroy(&mutexEXEC);
	sem_destroy(&mutexBlockKM);
	sem_destroy(&mutexREADY);
	sem_destroy(&mutexNEW);
	sem_destroy(&mutexEXIT);
	sem_destroy(&hayAlgunCPU);
	sem_destroy(&mutexListaCPUs);
	sem_destroy(&mutexListaConsolas);
	queue_destroy(nuevo);
	queue_destroy(ready);
	queue_destroy(exec);
	queue_destroy(block);
	queue_destroy(blockKM);
	queue_destroy(fin);
	list_destroy_and_destroy_elements(listacpus,(void*) _cpuDestroy);
	list_destroy_and_destroy_elements(listaConsolas,(void*) _consolaDestroy);
	log_destroy(logs);
	close(socketMSP);
	free(parm);
}

int recibirDatoDeMSP(int size,void** dato){
	*dato = malloc(size);
	if (recv (socketMSP, dato, size, MSG_WAITALL) <= 0){
		printf(" \n Se cerro la MSP abortando ... \n");
		exit(0);
	}
	return 1;
}

void manejoErrorLoader(int socket){
	eliminarConsolaDeLaLista(socket);
	close(socket);
	FD_CLR(socket,&conjuntoMaestroLoader);
}

void manejoErrorPlanificador(int i,thread_parm_t* parm){
	sem_wait(&mutexListaCPUs);
	buscadorCpu = i;
	t_cpu*cpu = list_find(listacpus, (void*)_is_CPUBuscado);
	int tid = cpu->tidactual;
	log_info(logs,"Se interrumpió CPU en socket %d",i);
	if(tid == 0){
		printf("\nHubo un problema con un CPU que ejecutaba HILO KERNEL. ABORTANDO....\n");
		while(!list_is_empty(listaConsolas)){
			t_consola* consolaAAvisar = list_get(listaConsolas,0);
			avisarErrorAConsola(problemaCPUHiloKernel,consolaAAvisar->socket);
			buscaYEliminaEnTodosLadosSegun(consolaAAvisar->pid);
			list_remove_and_destroy_element(listaConsolas,0, (void*)_consolaDestroy);
			mostrarEstadoColas();
		}
		eliminarDeMemoria(0,0);
		finalizarEjecucion(parm);
		exit(0);
	}else if(tid == -1){
		buscadorCpu = i;
		list_remove_and_destroy_by_condition(listacpus,(void*) _is_CPUBuscado,(void*) _cpuDestroy);
		printf("tam lista cpu %d",list_size(listacpus));
		fflush(stdout);
	}else{
		log_error(logs,"Se interrumpio el proceso con el PID: %d debido a que la CPU  en el socket: %d esta caida",tid,i);
		tcb* tcbAEliminar = buscarTCBen(exec,tid,&mutexEXEC,&hayAlgoEnExec);
		agregarAExit(tcbAEliminar);
		buscadorPorPid = tcbAEliminar->pid;
		mostrarEstadoColas();
		t_consola* consolaAAvisar = list_find(listaConsolas,(void*) _esConsolaPorPid);
		if(consolaAAvisar != NULL){
			avisarErrorAConsola(seInterrumpioCPU,consolaAAvisar->socket);
		}
		//elimina segmento de Stk del tcbAEliminar
		eliminarDeMemoria(tcbAEliminar->pid,tcbAEliminar->X);

		//busca a los hijos de ese tcb y los mata tambien
		buscarYEliminarHijosDe(tcbAEliminar->tid);

		//se fija si tiene padre, si no tiene elimina el segmento de codigo
		if(tcbAEliminar->ppid == -1){
			eliminarDeMemoria(tcbAEliminar->pid,tcbAEliminar->M);
		}
	}
	sem_post(&mutexListaCPUs);
	close(i);
	FD_CLR(i,&conjuntoMaestroPlanificador);
}

int recibirDatoLoader(int socket, int size, void** dato){
	*dato = malloc(size);
	if (recv (socket, dato, size, MSG_WAITALL) <= 0){
		manejoErrorLoader(socket);
		return 0;
	}
	return 1;
}

int recibirDatoPlanificador(int socket, int size, void** dato,thread_parm_t*parm){
	*dato = malloc(size);
	if (recv (socket, dato, size, MSG_WAITALL) <= 0){
		manejoErrorPlanificador(socket,parm);
		return 0;
	}
	return 1;
}
