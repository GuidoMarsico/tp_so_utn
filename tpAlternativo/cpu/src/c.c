#include "analizadorDeLinea.h"


void ejecutarTCB(tcb* TCB, int quantum){
	comienzoEjecucionPanel(TCB,quantum);
	termino = 0;
	int contador = 0;
	char* instruccion = string_new();
	while(termino == 0){
		if(TCB->kernelMode == 0){
			instruccion = leerDeMemoria(TCB->pid,TCB->P,4);
		}else{
			instruccion = leerDeMemoria(0,TCB->P,4);
		}
		TCB->P = sumar(4,TCB->P);
		TCB = analizarInstruccion(instruccion, TCB);
		contador++;
		if(TCB->kernelMode == 0 && contador == quantum){
			if(_esINTE(instruccion) || _esXXXX(instruccion)){
			}else{
				termino = 1;
				char* finDeQuantum = crearString("finq");
				send(socketKernel,&soyCPU,sizeof(int),0);
				recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
				send(socketKernel,finDeQuantum,strlen(finDeQuantum)+1,0);
				mandarTCBAlKernel(TCB);
				free(finDeQuantum);
			}
		}
		if(_esINTE(instruccion) || _esXXXX(instruccion)){
			termino = 1;
		}
	}
	free(instruccion);
}

int main(void) {
	inicializar_panel(CPU,"/home/utnso/git/tp-2014-2c-rafagadeterror/tpAlternativo/cpu/");
	thread_parm_t *parm = NULL;
	parm = malloc(sizeof(thread_parm_t));
	t_config* config = config_create(refcpuConfig);
	logs = log_create("logCPU", "cpu.c", 1, LOG_LEVEL_TRACE);

	parm->ipKernel = devolvemeElValorDe("IPKERNEL",config);
	parm->ipMSP = devolvemeElValorDe("IPMSP",config);
	parm->puertoKernel = devolvemeElValorDe("PUERTOKERNEL",config);
	parm->puertoMSP = devolvemeElValorDe("PUERTOMSP",config);
	parm->retardo = devolvemeElValorEnteroDe("RETARDO",config);

	retardo = parm->retardo/1000;
	soyCPU=1;
	rpaKernel=0;

	printf(KBLU"\nLa configuración del CPU es:\n");
	printf("PuertoMSP:%s\tIpMSP:%s\n",parm->puertoMSP,parm->ipMSP);
	printf("PuertoKERNEL:%s\tIpKERNEL:%s\n",parm->puertoKernel,parm->ipKernel);
	printf("Retardo:%d\n\n"KWHT,parm->retardo);

	tcb* TCBRecibido = malloc(sizeof(tcb));
	char* libre = crearString("lcpu");
	char* dameQuantum = crearString("dqua");
	sefl = crearString("sefl");
	sefe = crearString("sefe");
	int32_t quantumRecibido;
	socketKernel = conectarKernel(parm->ipKernel, parm->puertoKernel, logs);
	socketMSP = conectarMSP(parm->ipMSP, parm->puertoMSP, logs);
	t_stream* stream = malloc(sizeof(t_stream));
	abor = crearString("abor");

	while(1){
		fin_ejecucion();
		char paquete[sizeof(tcb)];
		send(socketKernel,&soyCPU,sizeof(int),0);
		recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
		send(socketKernel, libre, strlen(libre)+1, 0);
		desp:
		if(recibir(socketKernel,sizeof(tcb),(void*)&paquete) > 0){
			stream->datos=paquete;
			TCBRecibido=desserializadorTCB(stream);
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel, dameQuantum, strlen(dameQuantum)+1,0);
			recibir(socketKernel,sizeof(int32_t),(void*)&quantumRecibido);
			ejecutarTCB(TCBRecibido, quantumRecibido);
		}else{
			goto desp;
		}
	}
	free(parm);
	free(libre);
	free(dameQuantum);
	free(sefe);
	free(sefl);
	close(socketKernel);
	close(socketMSP);
	free(TCBRecibido);
	free(stream);
	free(abor);
	log_destroy(logs);
	return EXIT_SUCCESS;
}
