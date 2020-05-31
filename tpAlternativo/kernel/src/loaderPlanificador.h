#include "colasKernel.h"

int escribirEnMemoria(char* contenido, int32_t pid, u_int32_t dir, int tamCod){
	int opcion = 4;
	send(socketMSP, &opcion, sizeof(int), 0);
	escribir_enviar(pid,dir,tamCod,contenido);
	int respuesta = 0;
	recv(socketMSP,&respuesta,sizeof(int),MSG_WAITALL);
	if (respuesta == -1){
		printf("\nSEGMENTATION FAULT:");
		printf("\nLa MSP no logró escribir en memoria.\n");
		return -1;
	}
	return 0;
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
		recv(socketMSP,contenido,tamanio+1,MSG_WAITALL);
	}
	return contenido;
}

u_int32_t reservarDireccion(int tamanio, int32_t pid){
	u_int32_t dir;
	int opcion = 1;
	send(socketMSP, &opcion, sizeof(int), 0);
	reservar_destruir_enviar(pid,tamanio);//syscalls,base
	recibirDatoDeMSP(sizeof(u_int32_t),(void*)&dir);
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
	hilo->estaBloq = -1;
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
