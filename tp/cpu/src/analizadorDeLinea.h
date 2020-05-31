#include "serializadorCPU.h"

char* abor;

void abortar(){
	send(socketKernel,abor,strlen(abor)+1,0);
	exit(0);
}

int obtenerValorDeMemoriaDe(char* reg, tcb* thread){
	int num;
	if(string_equals_ignore_case(reg,"A")){
		num = thread->A;
	}else if(string_equals_ignore_case(reg,"B")){
		num = thread->B;
	}else if(string_equals_ignore_case(reg,"C")){
		num = thread->C;
	}else if(string_equals_ignore_case(reg,"D")){
		num = thread->D;
	}else if(string_equals_ignore_case(reg,"E")){
		num = thread->E;
	}else if(string_equals_ignore_case(reg,"M")){
		num = thread->M;
	}else if(string_equals_ignore_case(reg,"P")){
		num = thread->P;
	}else if(string_equals_ignore_case(reg,"S")){
		num = thread->S;
	}else if(string_equals_ignore_case(reg,"X")){
		num = thread->X;
	}else{
		abortar();
	}
	return num;
}

void cargarNumeroEn(int num, char* reg, tcb* thread){
	if(string_equals_ignore_case(reg,"A")){
		thread->A = num;
	}else if(string_equals_ignore_case(reg,"B")){
		thread->B = num;
	}else if(string_equals_ignore_case(reg,"C")){
		thread->C = num;
	}else if(string_equals_ignore_case(reg,"D")){
		thread->D = num;
	}else if(string_equals_ignore_case(reg,"E")){
		thread->E = num;
	}else if(string_equals_ignore_case(reg,"M")){
		thread->M = num;
	}else if(string_equals_ignore_case(reg,"P")){
		thread->P = num;
	}else if(string_equals_ignore_case(reg,"S")){
		thread->S = num;
	}else if(string_equals_ignore_case(reg,"X")){
		thread->X = num;
	}else{
		abortar();
	}
}

tcb* analizarInstruccion(char* instruccion, tcb* hilo){

	if( _esLOAD(instruccion)){
		char* registro = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		char* algo = leerDeMemoria(hilo->pid,hilo->P,4);
		int numero = (*((int *)algo)); //lo que hace es castea tod0s los chars del char* en ints y despues le pide el contenido al int* o sea un int
		printf("NUMERO: %d",numero);
		hilo->P = sumar(4,hilo->P);
		cargarNumeroEn(numero,registro,hilo);

	}else if( _esGETM(instruccion)){
		char* registro1 = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		char* registro2 = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		int valor = obtenerValorDeMemoriaDe(registro2, hilo);
		char* algo = leerDeMemoria(hilo->pid,hilo->M+valor,4);
		int numerito = (*((int *)algo));
		cargarNumeroEn(numerito, registro1, hilo);

	}else if( _esSETM(instruccion)){
		char* algo = leerDeMemoria(hilo->pid,hilo->P,4);
		int numero = (*((int *)algo));
		hilo->P = sumar(4,hilo->P);
		char* registro1 = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		char* registro2 = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		int valor1 = obtenerValorDeMemoriaDe(registro1, hilo);
		int valor2 = obtenerValorDeMemoriaDe(registro2, hilo);
		char* contenido = leerDeMemoria(hilo->pid,valor2,numero);
		escribirEnMemoria(contenido,hilo->P,valor1,numero);

	}else if( _esMOVR(instruccion)){
		char* registro1 = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		char* registro2 = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		int valor = obtenerValorDeMemoriaDe(registro2, hilo);
		cargarNumeroEn(valor, registro1, hilo);

	}else if( _esADDR(instruccion)){
		char* registro1 = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		char* registro2 = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		int valor1 = obtenerValorDeMemoriaDe(registro1, hilo);
		int valor2 = obtenerValorDeMemoriaDe(registro2, hilo);
		hilo->A = valor1 + valor2;

	}else if(_esSUBR(instruccion)){
		char* registro1 = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		char* registro2 = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		int valor1 = obtenerValorDeMemoriaDe(registro1, hilo);
		int valor2 = obtenerValorDeMemoriaDe(registro2, hilo);
		hilo->A = valor1 - valor2;

	}else if( _esMULR(instruccion)){
		char* registro1 = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		char* registro2 = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		int valor1 = obtenerValorDeMemoriaDe(registro1, hilo);
		int valor2 = obtenerValorDeMemoriaDe(registro2, hilo);
		hilo->A = valor1 * valor2;

	}else if( _esMODR(instruccion)){
		char* registro1 = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		char* registro2 = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		int valor1 = obtenerValorDeMemoriaDe(registro1, hilo);
		int valor2 = obtenerValorDeMemoriaDe(registro2, hilo);
		if(valor2 == 0){
			send(socketKernel,"div0",5,0);
		}else{
			hilo->A = valor1 % valor2;
		}

	}else if( _esDIVR(instruccion)){
		char* registro1 = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		char* registro2 = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		int valor1 = obtenerValorDeMemoriaDe(registro1, hilo);
		int valor2 = obtenerValorDeMemoriaDe(registro2, hilo);
		if(valor2 == 0){
			send(socketKernel,"div0",5,0);
		}else{
			hilo->A = valor1/valor2;
		}

	}else if( _esINCR(instruccion)){
		char* registro = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		int valor = obtenerValorDeMemoriaDe(registro, hilo);
		valor++;
		cargarNumeroEn(valor,registro,hilo);

	}else if( _esDECR(instruccion)){
		char* registro = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		int valor = obtenerValorDeMemoriaDe(registro, hilo);
		valor--;
		cargarNumeroEn(valor,registro,hilo);

	}else if( _esCOMP(instruccion)){
		char* registro1 = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		char* registro2 = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		int valor1 = obtenerValorDeMemoriaDe(registro1, hilo);
		int valor2 = obtenerValorDeMemoriaDe(registro2, hilo);
		hilo->A = (valor1 == valor2);

	}else if( _esCGEQ(instruccion)){
		char* registro1 =  leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		char* registro2 = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		int valor1 = obtenerValorDeMemoriaDe(registro1, hilo);
		int valor2 = obtenerValorDeMemoriaDe(registro2, hilo);
		hilo->A = (valor1 >= valor2);

	}else if( _esCLEQ(instruccion)){
		char* registro1 = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		char* registro2 =  leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		int valor1 = obtenerValorDeMemoriaDe(registro1, hilo);
		int valor2 = obtenerValorDeMemoriaDe(registro2, hilo);
		hilo->A = (valor1 <= valor2);

	}else if( _esGOTO(instruccion)){
		char* registro =  leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = obtenerValorDeMemoriaDe(registro, hilo);

	}else if( _esJMPZ(instruccion)){
		char* algo = leerDeMemoria(hilo->pid,hilo->P,4);
		int numero = (*((int *)algo));
		hilo->P = sumar(4,hilo->P);
		if(hilo->A == 0){
			hilo->P = numero;
		}

	}else if( _esJPNZ(instruccion)){
		char* algo = leerDeMemoria(hilo->pid,hilo->P,4);
		int numero = (*((int *)algo));
		hilo->P = sumar(4,hilo->P);
		if(hilo->A != 0){
			hilo->P = numero;
		}

	}else if( _esINTE(instruccion)){
		char* algo = leerDeMemoria(hilo->pid,hilo->P,4);
		u_int32_t direccion = (*((u_int32_t *)algo));
		hilo->P = sumar(4,hilo->P);
		printf("DIRECCION SYSCALLS: %d\n",direccion);
		printf("DIRECCION SYSCALLS: %x\n",direccion);
		hilo->dirSysCall = direccion;
		char* inte = crearString("inte");
		send(socketKernel,inte,strlen(inte)+1,0);
		free(inte);
		mandarTCBAlKernel(hilo);

		//---------INSTRUCCIONES PROTEGIDAS (sin parametros)-----------//
	}else if( _esMALC(instruccion)){
		//reserva una cantidad de memoria segun el valor del registro A, la direccion se guarda en el registro A
		if(!hilo->kernelMode){
			abortar();
		}else{
			int ok;
			int32_t pid;
			send(socketKernel,"malc",5,0);
			recibir(socketKernel,sizeof(int32_t),(void*)&pid);
			ok = reservarDireccion(hilo->A,pid);
			send(socketKernel,&ok,sizeof(int),0);
			if(ok!=-1){
				hilo->A = ok;
			}
		}

	}else if( _esFREE(instruccion)){
		if(!hilo->kernelMode){
			abortar();
		}else{
			int ok;
			int32_t pid;
			send(socketKernel,"free",5,0);
			recibir(socketKernel,sizeof(int32_t),(void*)&pid);
			ok = eliminarDeMemoria(pid,hilo->A);
			send(socketKernel,&ok,sizeof(int),0);
		}
		//TODO: if(ok != -1) hilo->A = -1; ???????

	}else if( _esINNN(instruccion)){
		if(!hilo->kernelMode){
			abortar();
		}else{
			//le manda al kernel un msj para que le pida por consola un numero
			//recibe el numero desde el kernel
			//se almacena el numero en el registro A (invoca al servicio correspondiente en el kernel)
			char* mensaje = crearString("innn");
			send(socketKernel,mensaje,strlen(mensaje)+1,0);
			int num = 0;
			recibir(socketKernel,sizeof(int),(void*)&num);
			send(socketKernel,&hilo->pid,sizeof(int32_t),0);
			recibir(socketKernel,sizeof(int),(void*)&num);
			hilo->A = num;
			free(mensaje);
		}

	}else if( _esINNC(instruccion)){
		if(!hilo->kernelMode){
			abortar();
		}else{
			//le manda al kernel un msj para que le pida por consola una cadena no mas larga de lo que indica el registro B
			//recibe la cadena desde el kernel
			char* mensaje = crearString("innc");
			send(socketKernel,mensaje,strlen(mensaje)+1,0);
			int num = 0;
			recibir(socketKernel,sizeof(int),(void*)&num);
			send(socketKernel,&hilo->pid,sizeof(int32_t),0);
			recibir(socketKernel,sizeof(int),(void*)&num);
			send(socketKernel,&hilo->B,sizeof(int32_t),0);
			char cad[hilo->B];
			int tamanio = hilo->B+1;
			recv(socketKernel,cad,tamanio,0);
			escribirEnMemoria(cad,hilo->pid,hilo->A,hilo->B);
			free(mensaje);
		}

	}else if( _esOUTN(instruccion)){
		if(!hilo->kernelMode){
			abortar();
		}else{
			//imprime por consola el numero almacenado en el registro A (invoca al servicio correspondiente en el kernel)
			char* mensaje = crearString("outn");
			send(socketKernel,mensaje,strlen(mensaje)+1,0);
			int num = 0;
			recibir(socketKernel,sizeof(int),(void*)&num);
			send(socketKernel,&hilo->pid,sizeof(int32_t),0);
			recibir(socketKernel,sizeof(int),(void*)&num);
			num = hilo->A;
			send(socketKernel,&num,sizeof(int),0);
			free(mensaje);
		}

	}else if( _esOUTC(instruccion)){
		if(!hilo->kernelMode){
			abortar();
		}else{
			//imprime por consola la cadena de tamaño indicado por B alojada en la direccion apuntada por A (invoca al servicio correspondiente en el kernel)
			char* mensaje = crearString("outc");
			send(socketKernel,mensaje,strlen(mensaje)+1,0);
			int num = 0;
			recibir(socketKernel,sizeof(int),(void*)&num);
			send(socketKernel,&hilo->pid,sizeof(int32_t),0);
			recibir(socketKernel,sizeof(int),(void*)&num);
			send(socketKernel,&hilo->B,sizeof(int32_t),0);
			recibir(socketKernel,sizeof(int),(void*)&num);
			char* cadena = leerDeMemoria(hilo->pid,hilo->A,hilo->B);
			send(socketKernel,cadena,hilo->B+1,0);
			printf("\nCadena enviada: %s\n",cadena);
		}

	}else if( _esCREA(instruccion)){
		if(!hilo->kernelMode){
			abortar();
		}else{
			int ok;
			send(socketKernel,"crea",5,0);
			recibir(socketKernel,sizeof(int),(void*)&ok);
			t_stream* stream = serializadorTcb(hilo);
			send(socketKernel, stream->datos, sizeof(tcb),0);
			int32_t tidRecibido;
			recibir(socketKernel,sizeof(int),(void*)&ok);
			if(ok != 1){
				//TODO: manejo de error (aborta o vuelve como lcpu???)
			}else{
				send(socketKernel,&ok,sizeof(int),0);
				recibir(socketKernel,sizeof(int32_t),(void*)&tidRecibido);
				hilo->A = tidRecibido;
			}
		}

	}else if( _esJOIN(instruccion)){
		if(!hilo->kernelMode){
			abortar();
		}else{
			//TODO: le manda al kernel que bloquee al hilo que esta en la cola BLOCKKM hasta que el hilo con el tid
			//que diga hilo->A termine de ejecutarse.
		}

	}else if( _esBLOK(instruccion)){
		if(!hilo->kernelMode){
			abortar();
		}else{
			//TODO: bloquea el programa que ejecuto la llamada al sistema hasta que el recurso apuntado por B se libere
		}

	}else if( _esWAKE(instruccion)){
		if(!hilo->kernelMode){
			abortar();
		}else{
			//TODO: desbloquea el primer programa bloqueado por el recurso apuntado por B
			//la evaluacion y decision de si el recurso esta libre o no es hecha por SIGNAL (precompilada)
		}

	}else if( _esSHIF(instruccion)){
		char* algo = leerDeMemoria(hilo->pid,hilo->P,4);
		int numero = (*((int *)algo));
		hilo->P = sumar(4,hilo->P);
		char* registro = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		int valor = obtenerValorDeMemoriaDe(registro, hilo);
		if(numero < 0){
			valor = valor << abs(numero);
		}else{
			valor = valor >> numero;
		}
		cargarNumeroEn(valor,registro, hilo);

	}else if( _esNOPP(instruccion)){
		//no hace nada...

	}else if( _esPUSH(instruccion)){
		char* algo = leerDeMemoria(hilo->pid,hilo->P,4);
		int numero = (*((int *)algo));
		hilo->P = sumar(4,hilo->P);
		char* registro = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		//apila los primeros bytes (segun el numero) del registro al stack
		int valor = obtenerValorDeMemoriaDe(registro, hilo);
		char* contenido = leerDeMemoria(hilo->pid,valor,numero);
		escribirEnMemoria(contenido,hilo->pid,hilo->S,numero);
		hilo->S = sumar(numero,hilo->S);

	}else if( _esTAKE(instruccion)){
		char* algo = leerDeMemoria(hilo->pid,hilo->P,4);
		int numero = (*((int *)algo));
		hilo->P = sumar(4,hilo->P);
		char* registro = leerDeMemoria(hilo->pid,hilo->P,1);
		hilo->P = sumar(1,hilo->P);
		//desapila los primeros bytes (segun el numero) del stack al registro
		int valor = obtenerValorDeMemoriaDe(registro, hilo);
		char* contenido = leerDeMemoria(hilo->pid,hilo->S,numero);
		escribirEnMemoria(contenido,hilo->pid,valor,numero);
		hilo->S = restar(numero,hilo->S);

	}else if( _esXXXX(instruccion)){
		char* finDeEjec = crearString("xxxx");
		send(socketKernel,finDeEjec,strlen(finDeEjec)+1,0);
		free(finDeEjec);
		mandarTCBAlKernel(hilo);

	}else{
		log_error(logs,"La instrucción leida es inválida");
		abortar();
	}
	sleep(retardo);
	return hilo;
}
