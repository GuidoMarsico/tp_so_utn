#include "planificador.h"

char* levantarSysCalls(char* path, int* tam){
	uint32_t size;
	FILE *binario;
	char* codigo;
	if ((binario = fopen(path, "r")) == NULL){
		printf("\n No existe la ruta %s\n",path);
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
	*tam = size;
	return codigo;
}


void crearHiloKernel(thread_parm_t* parm){
	u_int32_t dirCod;
	char* codigo;
	int tamArchivo;
	codigo = levantarSysCalls(parm->syscalls,&tamArchivo);
	dirCod = reservarDireccion(tamArchivo,0);
	printf("Dirección código: 0x%x\n", dirCod);
	if(dirCod != -1){
		tcb* hiloKernel = crearTCB(1,dirCod,-1,tamArchivo,0);
		escribirEnMemoria(codigo,0,dirCod,tamArchivo);
		agregarABlockKernelMode(hiloKernel);
	}
}

void arranque(thread_parm_t* parm){
	sem_init(&mutexNEW, 0, 1);
	sem_init(&mutexREADY, 0, 1);
	sem_init(&mutexEXEC, 0, 1);
	sem_init(&mutexEXIT, 0, 1);
	sem_init(&mutexBLOQ, 0, 1);
	sem_init(&hayAlgoEnNew, 0, 0);
	sem_init(&hayAlgoEnReady, 0, 0);
	sem_init(&hayAlgoEnExec, 0, 0);
	sem_init(&hayAlgoEnExit, 0, 0);
	sem_init(&hayAlgoEnBLOQ, 0, 0);
	sem_init(&mutexBlockKM, 0, 1);
	sem_init(&hayAlgoEnBlockKM, 0, 0);
	sem_init(&hayAlgunCPU, 0, 0);
	sem_init(&mutexListaCPUs, 0, 1);
	sem_init(&mutexListaConsolas, 0, 1);
	crearColas();
	socketMSP = conectarMSP(parm->ipMSP,parm->puertoMSP, logs);
	int inicio =0;
	send(socketMSP,&inicio,sizeof(int),0);
	crearHiloKernel(parm);
	mostrarEstadoColas();
}

void* hiloBloq(){
	pasarDeBlockKMAReady();
	pthread_exit(NULL);
}

void* hiloAsignarCpu(){
	while(1){
		int valor, valor2;
		sem_getvalue(&hayAlgoEnNew,&valor);
		if(valor > 0){
			sleep(1);
			mostrarEstadoColas();
			pasarDeNewAReady();
			mostrarEstadoColas();
		}
		sem_getvalue(&hayAlgoEnReady,&valor2);
		if(valor2 > 0 && !list_is_empty(listacpus)){
			printf("\n Cantidad de TCB en Ready %d\n",valor2);
			sem_wait(&hayAlgunCPU);
			buscadorPorTID = -1;
			sleep(2);
			t_cpu *cpuAAsignar = list_find(listacpus, (void*) _esCPUPorTID);
			if(cpuAAsignar== NULL){
			}else{
				sem_wait(&hayAlgoEnReady);
				tcb *tcbAsignado= queue_pop(ready);
				if(tcbAsignado != NULL){
					cpuAAsignar->tidactual = tcbAsignado->tid;
					t_stream* stream = serializadorTcb(tcbAsignado);
					printf("imprimiendo tcb:");
					printf(" Pid %d ",tcbAsignado->pid);
					printf(" km  %d",tcbAsignado->kernelMode);
					agregarAExec(tcbAsignado);
					mostrarEstadoColas();
					fflush(stdout);
					send(cpuAAsignar->socketCPU,stream->datos,sizeof(tcb),0);
				}
			}
		}
	}
	pthread_exit(NULL);
}

int main(void) {
	logs = log_create("logKernel", "kernel.c", 1, LOG_LEVEL_TRACE); //LOG
	thread_parm_t *parm = NULL;
	parm = malloc(sizeof(thread_parm_t));
	t_config* config = config_create(refKernelConfig);

	parm->ipMSP = devolvemeElValorDe("IPMSP",config);
	parm->puertoMSP = devolvemeElValorDe("PUERTOMSP",config);
	parm->puertoCPU = devolvemeElValorDe("PUERTOCPU",config);
	parm->puertoConsola = devolvemeElValorDe("PUERTOCONSOLA",config);
	parm->quantum = devolvemeElValorEnteroDe("QUANTUM",config);
	parm->tamanioStack = devolvemeElValorEnteroDe("TAMSTACK",config);
	parm->syscalls = devolvemeElValorDe("SYSCALLS",config);

	printf(KBLU "\nLa configuración del Kernel es:\n");
	printf("IpMSP:%s\tPuertoMSP:%s\n",parm->ipMSP,parm->puertoMSP);
	printf("PuertoCPU:%s\tPuertoConsola:%s\n",parm->puertoCPU, parm->puertoConsola);
	printf("Quantum:%i\tTamanioStack:%i\n", parm->quantum,parm->tamanioStack);
	printf("UbicacionSysCalls:%s\n\n" KNRM,parm->syscalls);

	pthread_t loader;
	pthread_t planificador;
	pthread_t hiloBloqueo;
	pthread_t hiloAsignar;

	arranque(parm);

	printf("Iniciando hilos\n");
	pthread_create(&loader,NULL,hiloLoader,(void*)parm);
	pthread_create(&planificador,NULL,hiloPlanificador,(void*)parm);
	pthread_create(&hiloBloqueo,NULL,hiloBloq,NULL);
	pthread_create(&hiloAsignar,NULL,hiloAsignarCpu,NULL);

	pthread_join(loader,NULL);
	pthread_join(planificador,NULL);
	pthread_join(hiloBloqueo,NULL);
	pthread_join(hiloAsignar,NULL);

	finalizarEjecucion(parm);

	config_destroy(config);

	return EXIT_SUCCESS;
}
