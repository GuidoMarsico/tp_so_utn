#include "AtencionLoaderYPlanificador.h"

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
	u_int32_t dirCod, dirStk;
	char* codigo;
	int tamArchivo;
	codigo = levantarSysCalls(parm->syscalls,&tamArchivo);
	dirCod = reservarDireccion(tamArchivo,0);
	if(dirCod != -1){
		dirStk = reservarDireccion(parm->tamanioStack,0);
		if(dirStk != -1){
			tcb* hiloKernel = crearTCB(1,dirCod,dirStk,tamArchivo,0);
			int result = escribirEnMemoria(codigo,0,dirCod,tamArchivo);
			if (result == -1){
				printf("\nNo pudo escribir en memoria las SYSCALLS. SEGMENTATION FAULT\n");
				exit(0);
			}
			agregarABlockKernelMode(hiloKernel);
		}else{
			printf("No se pudo reservar memoria para el segmento de stack del KM");
			exit(0);
		}
	}else{
		printf("No se pudo reservar memoria para el segmento de código del KM");
		exit(0);
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
	//mostrarEstadoColas();
	mostrarColasPanel();
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
			//mostrarEstadoColas();
			mostrarColasPanel();
			pasarDeNewAReady();
			//mostrarEstadoColas();
			mostrarColasPanel();
		}
		sleep(0.5);
		sem_getvalue(&hayAlgoEnReady,&valor2);
		if(valor2 > 0 && !list_is_empty(listacpus)){
			printf("\n Cantidad de TCB en Ready %d\n",valor2);
			sem_wait(&hayAlgunCPU);
			buscadorPorTID = -1;
			t_cpu *cpuAAsignar = list_find(listacpus, (void*) _esCPUPorTID);
			if(cpuAAsignar== NULL){
			}else{
				sem_wait(&hayAlgoEnReady);
				sem_wait(&mutexREADY);
				tcb *tcbAsignado= queue_pop(ready);
				sem_post(&mutexREADY);
				if(tcbAsignado != NULL){
					cpuAAsignar->tidactual = tcbAsignado->tid;
					t_stream* stream = serializadorTcb(tcbAsignado);
					agregarAExec(tcbAsignado);
					//mostrarEstadoColas();
					mostrarColasPanel();
					fflush(stdout);
					send(cpuAAsignar->socketCPU,stream->datos,sizeof(tcb),0);
				}
			}
		}
	}
	pthread_exit(NULL);
}

int main(void) {
	inicializar_panel(KERNEL,"/home/utnso/git/tp-2014-2c-rafagadeterror/tpAlternativo/kernel/");
	logs = log_create("logKernel", "kernel.c", 1, LOG_LEVEL_TRACE); //LOG
	thread_parm_t *parm = NULL;
	parm = malloc(sizeof(thread_parm_t));
	t_config* config = config_create(refKernelConfig);

	parm->ipMSP = devolvemeElValorDe("IPMSP",config);
	parm->puertoMSP = devolvemeElValorDe("PUERTOMSP",config);
	parm->puertoEscucha = devolvemeElValorDe("PUERTOESCUCHA",config);
	parm->quantum = devolvemeElValorEnteroDe("QUANTUM",config);
	parm->tamanioStack = devolvemeElValorEnteroDe("TAMSTACK",config);
	parm->syscalls = devolvemeElValorDe("SYSCALLS",config);

	printf(KBLU "\nLa configuración del Kernel es:\n");
	printf("IpMSP:%s\tPuertoMSP:%s\n",parm->ipMSP,parm->puertoMSP);
	printf("PuertoESCUCHA:%s\n",parm->puertoEscucha);
	printf("Quantum:%i\tTamanioStack:%i\n", parm->quantum,parm->tamanioStack);
	printf("UbicacionSysCalls:%s\n\n" KNRM,parm->syscalls);

	pthread_t hiloatencionLoaderYPlanificador;
	pthread_t hiloBloqueo;
	pthread_t hiloAsignar;

	arranque(parm);

	printf("Iniciando hilos\n");
	pthread_create(&hiloatencionLoaderYPlanificador,NULL,hiloAtencionLP,(void*)parm);
	pthread_create(&hiloBloqueo,NULL,hiloBloq,NULL);
	pthread_create(&hiloAsignar,NULL,hiloAsignarCpu,NULL);

	pthread_join(hiloatencionLoaderYPlanificador,NULL);
	pthread_join(hiloBloqueo,NULL);
	pthread_join(hiloAsignar,NULL);

	finalizarEjecucion(parm);

	config_destroy(config);

	return EXIT_SUCCESS;
}
