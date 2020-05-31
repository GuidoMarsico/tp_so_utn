#include "serializadorCPU.h"

char* abor;

void abortar(){
	send(socketKernel,&soyCPU,sizeof(int),0);
	recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
	send(socketKernel,abor,strlen(abor)+1,0);
	exit(0);
}

void destructorParams(char* elem){
	free(elem);
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

char* sefl;
char* sefe;

tcb* analizarInstruccion(char* instruccion, tcb* hilo){

	if( _esLOAD(instruccion)){
		int32_t pid = 0;
		if(hilo->kernelMode == 0){
			pid = hilo->pid;
		}
		char* registro = leerDeMemoria(pid,hilo->P,1);
		if(registro == NULL){
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,sefl,5,0);
			termino = 1;
		}else{
			hilo->P = sumar(1,hilo->P);
			char* algo = leerDeMemoria(pid,hilo->P,4);
			if(algo == NULL){
				send(socketKernel,&soyCPU,sizeof(int),0);
				recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
				send(socketKernel,sefl,5,0);
				termino = 1;
			}else{
				int numero = (*((int *)algo)); //lo que hace es castea tod0s los chars del char* en ints y despues le pide el contenido al int* o sea un int
				hilo->P = sumar(4,hilo->P);
				t_list* params = list_create();
				list_add(params,registro);
				list_add(params,string_itoa(numero));
				printf(KYEL);
				ejecucion_instruccion("LOAD",params);
				printf(KWHT);
				cargarNumeroEn(numero,registro,hilo);
				list_destroy_and_destroy_elements(params,(void*)destructorParams);
			}
		}
	}else if( _esGETM(instruccion)){
		int32_t pid = 0;
		if(hilo->kernelMode == 0){
			pid = hilo->pid;
		}
		char* registro1 = leerDeMemoria(pid,hilo->P,1);
		if(registro1 == NULL){
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,sefl,5,0);
			termino = 1;
		}else{
			hilo->P = sumar(1,hilo->P);
			char* registro2 = leerDeMemoria(pid,hilo->P,1);
			if(registro2 == NULL){
				send(socketKernel,&soyCPU,sizeof(int),0);
				recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
				send(socketKernel,sefl,5,0);
				termino = 1;
			}else{
				hilo->P = sumar(1,hilo->P);
				int valor = obtenerValorDeMemoriaDe(registro2, hilo);
				char* algo = leerDeMemoria(pid, valor,4);
				if(algo == NULL){
					send(socketKernel,&soyCPU,sizeof(int),0);
					recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
					send(socketKernel,sefl,5,0);
					termino = 1;
				}else{
					int numerito2 = (*(int*)algo);
					int numerito;
					if(algo[0]<0){
						numerito = algo[0];
					}else{
						unsigned char* bytesnum = malloc(1);
						bytesnum = conversorIntByteCode(1,numerito2,bytesnum);
						numerito = (*(int*)bytesnum);
						free(bytesnum);
					}
					t_list* params = list_create();
					list_add(params,registro1);
					list_add(params,registro2);
					printf(KYEL);
					ejecucion_instruccion("GETM",params);
					printf(KWHT);
					cargarNumeroEn(numerito, registro1, hilo);
					list_destroy_and_destroy_elements(params,(void*)destructorParams);
				}
			}
		}

	}else if( _esSETM(instruccion)){
		int32_t pid = 0;
		if(hilo->kernelMode == 0){
			pid = hilo->pid;
		}
		char* algo = leerDeMemoria(pid,hilo->P,4);
		if(algo == NULL){
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,sefl,5,0);
			termino = 1;
		}else{
			int numero = (*((int *)algo));
			hilo->P = sumar(4,hilo->P);
			char* registro1 = leerDeMemoria(pid,hilo->P,1);
			if(registro1 == NULL){
				send(socketKernel,&soyCPU,sizeof(int),0);
				recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
				send(socketKernel,sefl,5,0);
				termino = 1;
			}else{
				hilo->P = sumar(1,hilo->P);
				char* registro2 = leerDeMemoria(pid,hilo->P,1);
				if(registro2 == NULL){
					send(socketKernel,&soyCPU,sizeof(int),0);
					recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
					send(socketKernel,sefl,5,0);
					termino = 1;
				}else{
					hilo->P = sumar(1,hilo->P);
					int valor1 = obtenerValorDeMemoriaDe(registro1, hilo);
					int valor2 = obtenerValorDeMemoriaDe(registro2, hilo);
					int result;
					if(valor2<0){
						char bytes[4];
						bytes[3] = (valor2 >> 24) & 0xFF;
						bytes[2] = (valor2 >> 16) & 0xFF;
						bytes[1] = (valor2 >> 8) & 0xFF;
						bytes[0] = valor2 & 0xFF;
						char* bytesnum = malloc(numero);
						if(numero >= 1){
							bytesnum[0] = bytes[0];
							if(numero >=2){
								bytesnum[1] = bytes[1];
								if(numero >=3){
									bytesnum[2] = bytes[2];
									if(numero == 4){
										bytesnum[3] = bytes[3];
									}else{
										bytesnum[3] = 0;
									}
								}else{
									bytesnum[2] = 0;
									bytesnum[3] = 0;
								}
							}else{
								bytesnum[1] = 0;
								bytesnum[2] = 0;
								bytesnum[3] = 0;
							}
						}
						result = escribirEnMemoria(bytesnum,pid,valor1,numero);
						free(bytesnum);
					}else{
						unsigned char* contenido = malloc(numero);
						contenido = conversorIntByteCode(numero,valor2,contenido);
						result = escribirEnMemoria(contenido,pid,valor1,numero);
						free(contenido);
					}
					if(result == -1){
						send(socketKernel,&soyCPU,sizeof(int),0);
						recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
						send(socketKernel,sefe,5,0);
						termino = 1;
					}
					t_list* params = list_create();
					list_add(params,string_itoa(numero));
					list_add(params,registro1);
					list_add(params,registro2);
					printf(KYEL);
					ejecucion_instruccion("SETM",params);
					printf(KWHT);
					list_destroy_and_destroy_elements(params,(void*)destructorParams);
				}
			}
		}

	}else if( _esMOVR(instruccion)){
		int32_t pid = 0;
		if(hilo->kernelMode == 0){
			pid = hilo->pid;
		}
		char* registro1 = leerDeMemoria(pid,hilo->P,1);
		if(registro1 == NULL){
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,sefl,5,0);
			termino = 1;
		}else{
			hilo->P = sumar(1,hilo->P);
			char* registro2 = leerDeMemoria(pid,hilo->P,1);
			if(registro2 == NULL){
				send(socketKernel,&soyCPU,sizeof(int),0);
				recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
				send(socketKernel,sefl,5,0);
				termino = 1;
			}else{
				hilo->P = sumar(1,hilo->P);
				int valor = obtenerValorDeMemoriaDe(registro2, hilo);
				t_list* params = list_create();
				list_add(params,registro1);
				list_add(params,registro2);
				printf(KYEL);
				ejecucion_instruccion("MOVR",params);
				printf(KWHT);
				cargarNumeroEn(valor, registro1, hilo);
				list_destroy_and_destroy_elements(params,(void*)destructorParams);
			}
		}

	}else if( _esADDR(instruccion)){
		int32_t pid = 0;
		if(hilo->kernelMode == 0){
			pid = hilo->pid;
		}
		char* registro1 = leerDeMemoria(pid,hilo->P,1);
		if(registro1 == NULL){
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,sefl,5,0);
			termino = 1;
		}else{
			hilo->P = sumar(1,hilo->P);
			char* registro2 = leerDeMemoria(pid,hilo->P,1);
			if(registro2 == NULL){
				send(socketKernel,&soyCPU,sizeof(int),0);
				recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
				send(socketKernel,sefl,5,0);
				termino = 1;
			}else{
				hilo->P = sumar(1,hilo->P);
				int valor1 = obtenerValorDeMemoriaDe(registro1, hilo);
				int valor2 = obtenerValorDeMemoriaDe(registro2, hilo);
				hilo->A = valor1 + valor2;
				t_list* params = list_create();
				list_add(params,registro1);
				list_add(params,registro2);
				printf(KYEL);
				ejecucion_instruccion("ADDR",params);
				printf(KWHT);
				list_destroy_and_destroy_elements(params,(void*)destructorParams);
			}
		}

	}else if(_esSUBR(instruccion)){
		char* registro1 = leerDeMemoria(hilo->pid,hilo->P,1);
		if(registro1 == NULL){
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,sefl,5,0);
			termino = 1;
		}else{
			hilo->P = sumar(1,hilo->P);
			char* registro2 = leerDeMemoria(hilo->pid,hilo->P,1);
			if(registro2 == NULL){
				send(socketKernel,&soyCPU,sizeof(int),0);
				recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
				send(socketKernel,sefl,5,0);
				termino = 1;
			}else{
				hilo->P = sumar(1,hilo->P);
				int valor1 = obtenerValorDeMemoriaDe(registro1, hilo);
				int valor2 = obtenerValorDeMemoriaDe(registro2, hilo);
				hilo->A = valor1 - valor2;
				t_list* params = list_create();
				list_add(params,registro1);
				list_add(params,registro2);
				printf(KYEL);
				ejecucion_instruccion("SUBR",params);
				printf(KWHT);
				list_destroy_and_destroy_elements(params,(void*)destructorParams);
			}
		}

	}else if( _esMULR(instruccion)){
		char* registro1 = leerDeMemoria(hilo->pid,hilo->P,1);
		if(registro1 == NULL){
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,sefl,5,0);
			termino = 1;
		}else{
			hilo->P = sumar(1,hilo->P);
			char* registro2 = leerDeMemoria(hilo->pid,hilo->P,1);
			if(registro2 == NULL){
				send(socketKernel,&soyCPU,sizeof(int),0);
				recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
				send(socketKernel,sefl,5,0);
				termino = 1;
			}else{
				hilo->P = sumar(1,hilo->P);
				int valor1 = obtenerValorDeMemoriaDe(registro1, hilo);
				int valor2 = obtenerValorDeMemoriaDe(registro2, hilo);
				hilo->A = valor1 * valor2;
				t_list* params = list_create();
				list_add(params,registro1);
				list_add(params,registro2);
				printf(KYEL);
				ejecucion_instruccion("MULR",params);
				printf(KWHT);
				list_destroy_and_destroy_elements(params,(void*)destructorParams);
			}
		}

	}else if( _esMODR(instruccion)){
		char* registro1 = leerDeMemoria(hilo->pid,hilo->P,1);
		if(registro1 == NULL){
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,sefl,5,0);
			termino = 1;
		}else{
			hilo->P = sumar(1,hilo->P);
			char* registro2 = leerDeMemoria(hilo->pid,hilo->P,1);
			if(registro2 == NULL){
				send(socketKernel,&soyCPU,sizeof(int),0);
				recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
				send(socketKernel,sefl,5,0);
				termino = 1;
			}else{
				hilo->P = sumar(1,hilo->P);
				int valor1 = obtenerValorDeMemoriaDe(registro1, hilo);
				int valor2 = obtenerValorDeMemoriaDe(registro2, hilo);
				if(valor2 == 0){
					send(socketKernel,&soyCPU,sizeof(int),0);
					recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
					send(socketKernel,"div0",5,0);
					termino = 1;
				}else{
					hilo->A = valor1 % valor2;
					t_list* params = list_create();
					list_add(params,registro1);
					list_add(params,registro2);
					printf(KYEL);
					ejecucion_instruccion("MODR",params);
					printf(KWHT);
					list_destroy_and_destroy_elements(params,(void*)destructorParams);
				}
			}
		}

	}else if( _esDIVR(instruccion)){
		char* registro1 = leerDeMemoria(hilo->pid,hilo->P,1);
		if(registro1 == NULL){
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,sefl,5,0);
			termino = 1;
		}else{
			hilo->P = sumar(1,hilo->P);
			char* registro2 = leerDeMemoria(hilo->pid,hilo->P,1);
			if(registro2 == NULL){
				send(socketKernel,&soyCPU,sizeof(int),0);
				recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
				send(socketKernel,sefl,5,0);
				termino = 1;
			}else{
				hilo->P = sumar(1,hilo->P);
				int valor1 = obtenerValorDeMemoriaDe(registro1, hilo);
				int valor2 = obtenerValorDeMemoriaDe(registro2, hilo);
				if(valor2 == 0){
					send(socketKernel,&soyCPU,sizeof(int),0);
					recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
					send(socketKernel,"div0",5,0);
					termino = 1;
				}else{
					hilo->A = valor1/valor2;
					t_list* params = list_create();
					list_add(params,registro1);
					list_add(params,registro2);
					printf(KYEL);
					ejecucion_instruccion("DIVR",params);
					printf(KWHT);
					list_destroy_and_destroy_elements(params,(void*)destructorParams);
				}
			}
		}

	}else if( _esINCR(instruccion)){
		int32_t pid = 0;
		if(hilo->kernelMode == 0){
			pid = hilo->pid;
		}
		char* registro = leerDeMemoria(pid,hilo->P,1);
		if(registro == NULL){
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,sefl,5,0);
			termino = 1;
		}else{
			hilo->P = sumar(1,hilo->P);
			int valor = obtenerValorDeMemoriaDe(registro, hilo);
			valor = valor + 1;
			t_list* params = list_create();
			list_add(params,registro);
			printf(KYEL);
			ejecucion_instruccion("INCR",params);
			printf(KWHT);
			cargarNumeroEn(valor,registro,hilo);
			list_destroy_and_destroy_elements(params,(void*)destructorParams);
		}

	}else if( _esDECR(instruccion)){
		int32_t pid = 0;
		if(hilo->kernelMode == 0){
			pid = hilo->pid;
		}
		char* registro = leerDeMemoria(pid,hilo->P,1);
		if(registro == NULL){
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,sefl,5,0);
			termino = 1;
		}else{
			hilo->P = sumar(1,hilo->P);
			int valor = obtenerValorDeMemoriaDe(registro, hilo);
			valor = valor - 1;
			t_list* params = list_create();
			list_add(params,registro);
			printf(KYEL);
			ejecucion_instruccion("DECR",params);
			printf(KWHT);
			cargarNumeroEn(valor,registro,hilo);
			list_destroy_and_destroy_elements(params,(void*)destructorParams);
		}

	}else if( _esCOMP(instruccion)){
		char* registro1 = leerDeMemoria(hilo->pid,hilo->P,1);
		if(registro1 == NULL){
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,sefl,5,0);
			termino = 1;
		}else{
			hilo->P = sumar(1,hilo->P);
			char* registro2 = leerDeMemoria(hilo->pid,hilo->P,1);
			if(registro2 == NULL){
				send(socketKernel,&soyCPU,sizeof(int),0);
				recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
				send(socketKernel,sefl,5,0);
				termino = 1;
			}else{
				hilo->P = sumar(1,hilo->P);
				int valor1 = obtenerValorDeMemoriaDe(registro1, hilo);
				int valor2 = obtenerValorDeMemoriaDe(registro2, hilo);
				hilo->A = (valor1 == valor2);
				t_list* params = list_create();
				list_add(params,registro1);
				list_add(params,registro2);
				printf(KYEL);
				ejecucion_instruccion("COMP",params);
				printf(KWHT);
				list_destroy_and_destroy_elements(params,(void*)destructorParams);
			}
		}

	}else if( _esCGEQ(instruccion)){
		char* registro1 =  leerDeMemoria(hilo->pid,hilo->P,1);
		if(registro1 == NULL){
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,sefl,5,0);
			termino = 1;
		}else{
			hilo->P = sumar(1,hilo->P);
			char* registro2 = leerDeMemoria(hilo->pid,hilo->P,1);
			if(registro2 == NULL){
				send(socketKernel,&soyCPU,sizeof(int),0);
				recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
				send(socketKernel,sefl,5,0);
				termino = 1;
			}else{
				hilo->P = sumar(1,hilo->P);
				int valor1 = obtenerValorDeMemoriaDe(registro1, hilo);
				int valor2 = obtenerValorDeMemoriaDe(registro2, hilo);
				hilo->A = (valor1 >= valor2);
				t_list* params = list_create();
				list_add(params,registro1);
				list_add(params,registro2);
				printf(KYEL);
				ejecucion_instruccion("CGEQ",params);
				printf(KWHT);
				list_destroy_and_destroy_elements(params,(void*)destructorParams);
			}
		}

	}else if( _esCLEQ(instruccion)){
		int32_t pid = 0;
		if(hilo->kernelMode == 0){
			pid = hilo->pid;
		}
		char* registro1 = leerDeMemoria(pid,hilo->P,1);
		if(registro1 == NULL){
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,sefl,5,0);
			termino = 1;
		}else{
			hilo->P = sumar(1,hilo->P);
			char* registro2 =  leerDeMemoria(pid,hilo->P,1);
			if(registro2 == NULL){
				send(socketKernel,&soyCPU,sizeof(int),0);
				recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
				send(socketKernel,sefl,5,0);
				termino = 1;
			}else{
				hilo->P = sumar(1,hilo->P);
				int valor1 = obtenerValorDeMemoriaDe(registro1, hilo);
				int valor2 = obtenerValorDeMemoriaDe(registro2, hilo);
				hilo->A = (valor1 <= valor2);
				t_list* params = list_create();
				list_add(params,registro1);
				list_add(params,registro2);
				printf(KYEL);
				ejecucion_instruccion("CLEQ",params);
				printf(KWHT);
				list_destroy_and_destroy_elements(params,(void*)destructorParams);
			}
		}

	}else if( _esGOTO(instruccion)){
		char* registro =  leerDeMemoria(hilo->pid,hilo->P,1);
		if(registro == NULL){
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,sefl,5,0);
			termino = 1;
		}else{
			hilo->P = obtenerValorDeMemoriaDe(registro, hilo);
			t_list* params = list_create();
			list_add(params,registro);
			printf(KYEL);
			ejecucion_instruccion("GOTO",params);
			printf(KWHT);
			list_destroy_and_destroy_elements(params,(void*)destructorParams);
		}

	}else if( _esJMPZ(instruccion)){
		char* algo = leerDeMemoria(hilo->pid,hilo->P,4);
		if(algo == NULL){
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,sefl,5,0);
			termino = 1;
		}else{
			int numero = (*((int *)algo));
			hilo->P = sumar(4,hilo->P);
			if(hilo->A == 0){
				hilo->P = numero;
			}
			t_list* params = list_create();
			list_add(params,string_itoa(numero));
			printf(KYEL);
			ejecucion_instruccion("JMPZ",params);
			printf(KWHT);
			list_destroy_and_destroy_elements(params,(void*)destructorParams);
		}

	}else if( _esJPNZ(instruccion)){
		int32_t pid = 0;
		if(hilo->kernelMode == 0){
			pid = hilo->pid;
		}
		char* algo = leerDeMemoria(pid,hilo->P,4);
		if(algo == NULL){
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,sefl,5,0);
			termino = 1;
		}else{
			int numero = (*((int *)algo));
			hilo->P = sumar(4,hilo->P);
			if(hilo->A != 0){
				hilo->P = numero;
			}
			t_list* params = list_create();
			list_add(params,string_itoa(numero));
			printf(KYEL);
			ejecucion_instruccion("JPNZ",params);
			printf(KWHT);
			list_destroy_and_destroy_elements(params,(void*)destructorParams);
		}

	}else if( _esINTE(instruccion)){
		char* algo = leerDeMemoria(hilo->pid,hilo->P,4);
		if(algo == NULL){
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,sefl,5,0);
			termino = 1;
		}else{
			u_int32_t direccion = (*((u_int32_t *)algo));
			hilo->P = sumar(4,hilo->P);
			hilo->dirSysCall = direccion;
			char* inte = crearString("inte");
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,inte,strlen(inte)+1,0);
			free(inte);
			mandarTCBAlKernel(hilo);
			t_list* params = list_create();
			list_add(params,string_itoa(direccion));
			printf(KYEL);
			ejecucion_instruccion("INTE",params);
			printf(KWHT);
			list_destroy_and_destroy_elements(params,(void*)destructorParams);
		}

		//---------INSTRUCCIONES PROTEGIDAS (sin parametros)-----------//
	}else if( _esMALC(instruccion)){
		//reserva una cantidad de memoria segun el valor del registro A, la direccion se guarda en el registro A
		if(!hilo->kernelMode){
			abortar();
		}else{
			int ok;
			int32_t pid;
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,"malc",5,0);
			recibir(socketKernel,sizeof(int32_t),(void*)&pid);
			ok = reservarDireccion(hilo->A,pid);
			send(socketKernel,&ok,sizeof(int),0);
			if(ok!=-1){
				hilo->A = ok;
			}
			t_list* params = list_create();
			char* nullable = crearString("NULL");
			list_add(params,nullable);
			printf(KRED);
			ejecucion_instruccion("MALC",params);
			printf(KWHT);
			list_destroy_and_destroy_elements(params,(void*)destructorParams);
		}

	}else if( _esFREE(instruccion)){
		if(!hilo->kernelMode){
			abortar();
		}else{
			int ok;
			int32_t pid;
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,"free",5,0);
			recibir(socketKernel,sizeof(int32_t),(void*)&pid);
			ok = eliminarDeMemoria(pid,hilo->A);
			send(socketKernel,&ok,sizeof(int),0);
			t_list* params = list_create();
			char* nullable = crearString("NULL");
			list_add(params,nullable);
			printf(KRED);
			ejecucion_instruccion("FREE",params);
			printf(KWHT);
			list_destroy_and_destroy_elements(params,(void*)destructorParams);
		}

	}else if( _esINNN(instruccion)){
		if(!hilo->kernelMode){
			abortar();
		}else{
			//le manda al kernel un msj para que le pida por consola un numero
			//recibe el numero desde el kernel
			//se almacena el numero en el registro A (invoca al servicio correspondiente en el kernel)
			char* mensaje = crearString("innn");
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,mensaje,strlen(mensaje)+1,0);
			int num = 0;
			recibir(socketKernel,sizeof(int),(void*)&num);
			send(socketKernel,&hilo->pid,sizeof(int32_t),0);
			recibir(socketKernel,sizeof(int),(void*)&num);
			hilo->A = num;
			free(mensaje);
			t_list* params = list_create();
			char* nullable = crearString("NULL");
			list_add(params,nullable);
			printf(KRED);
			ejecucion_instruccion("INNN",params);
			printf(KWHT);
			list_destroy_and_destroy_elements(params,(void*)destructorParams);
		}

	}else if( _esINNC(instruccion)){
		if(!hilo->kernelMode){
			abortar();
		}else{
			//le manda al kernel un msj para que le pida por consola una cadena no mas larga de lo que indica el registro B
			//recibe la cadena desde el kernel
			char* mensaje = crearString("innc");
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,mensaje,strlen(mensaje)+1,0);
			int num = 0;
			recibir(socketKernel,sizeof(int),(void*)&num);
			send(socketKernel,&hilo->pid,sizeof(int32_t),0);
			recibir(socketKernel,sizeof(int),(void*)&num);
			send(socketKernel,&hilo->B,sizeof(int32_t),0);
			int tamanio = 0;
			recv(socketKernel,&tamanio,sizeof(int),MSG_WAITALL);
			char* cad = malloc(tamanio+1);
			send(socketKernel,&num,sizeof(int),0);
			recv(socketKernel,cad,tamanio+1,0);
			int result = escribirEnMemoria(cad,hilo->pid,hilo->A,hilo->B);
			if (result == -1){
				send(socketKernel,&soyCPU,sizeof(int),0);
				recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
				send(socketKernel,sefe,5,0);
				termino = 1;
			}
			free(mensaje);
			t_list* params = list_create();
			char* nullable = crearString("NULL");
			list_add(params,nullable);
			printf(KRED);
			ejecucion_instruccion("INNC",params);
			printf(KWHT);
			list_destroy_and_destroy_elements(params,(void*)destructorParams);
		}

	}else if( _esOUTN(instruccion)){
		if(!hilo->kernelMode){
			abortar();
		}else{
			//imprime por consola el numero almacenado en el registro A (invoca al servicio correspondiente en el kernel)
			char* mensaje = crearString("outn");
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,mensaje,strlen(mensaje)+1,0);
			int num = 0;
			recibir(socketKernel,sizeof(int),(void*)&num);
			send(socketKernel,&hilo->pid,sizeof(int32_t),0);
			recibir(socketKernel,sizeof(int),(void*)&num);
			num = hilo->A;
			send(socketKernel,&num,sizeof(int),0);
			free(mensaje);
			t_list* params = list_create();
			char* nullable = crearString("NULL");
			list_add(params,nullable);
			printf(KRED);
			ejecucion_instruccion("OUTN",params);
			printf(KWHT);
			list_destroy_and_destroy_elements(params,(void*)destructorParams);
		}

	}else if( _esOUTC(instruccion)){
		if(!hilo->kernelMode){
			abortar();
		}else{
			//imprime por consola la cadena de tamaño indicado por B alojada en la direccion apuntada por A (invoca al servicio correspondiente en el kernel)
			char* mensaje = crearString("outc");
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,mensaje,strlen(mensaje)+1,0);
			int num = 0;
			recibir(socketKernel,sizeof(int),(void*)&num);
			send(socketKernel,&hilo->pid,sizeof(int32_t),0);
			recibir(socketKernel,sizeof(int),(void*)&num);
			send(socketKernel,&hilo->B,sizeof(int32_t),0);
			recibir(socketKernel,sizeof(int),(void*)&num);
			char* cadena = leerDeMemoria(hilo->pid,hilo->A,hilo->B);
			if(cadena == NULL){
				send(socketKernel,&soyCPU,sizeof(int),0);
				recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
				send(socketKernel,sefl,5,0);
				termino = 1;
			}else{
				send(socketKernel,cadena,hilo->B+1,0);
				t_list* params = list_create();
				char* nullable = crearString("NULL");
				list_add(params,nullable);
				printf(KRED);
				ejecucion_instruccion("OUTC",params);
				printf(KWHT);
				list_destroy_and_destroy_elements(params,(void*)destructorParams);
			}
		}

	}else if( _esCREA(instruccion)){
		if(!hilo->kernelMode){
			abortar();
		}else{
			int ok;
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,"crea",5,0);
			recibir(socketKernel,sizeof(int),(void*)&ok);
			t_stream* stream = serializadorTcb(hilo);
			send(socketKernel, stream->datos, sizeof(tcb),0);
			int32_t tidRecibido;
			recibir(socketKernel,sizeof(int),(void*)&ok);
			if(ok != 1){
				printf("\nHubo un error al intentar crear hilo hijo\n");
				termino = 1;
			}else{
				send(socketKernel,&ok,sizeof(int),0);
				recibir(socketKernel,sizeof(int32_t),(void*)&tidRecibido);
				hilo->A = tidRecibido;
				t_list* params = list_create();
				char* nullable = crearString("NULL");
				list_add(params,nullable);
				printf(KRED);
				ejecucion_instruccion("CREA",params);
				printf(KWHT);
				list_destroy_and_destroy_elements(params,(void*)destructorParams);
			}
		}

	}else if( _esJOIN(instruccion)){
		if(!hilo->kernelMode){
			abortar();
		}else{
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,"join",5,0);
			t_list* params = list_create();
			char* nullable = crearString("NULL");
			list_add(params,nullable);
			printf(KRED);
			ejecucion_instruccion("JOIN",params);
			printf(KWHT);
			list_destroy_and_destroy_elements(params,(void*)destructorParams);
		}

	}else if( _esBLOK(instruccion)){
		if(!hilo->kernelMode){
			abortar();
		}else{
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,"blok",5,0);
			t_list* params = list_create();
			char* nullable = crearString("NULL");
			list_add(params,nullable);
			printf(KRED);
			ejecucion_instruccion("BLOCK",params);
			printf(KWHT);
			list_destroy_and_destroy_elements(params,(void*)destructorParams);
		}

	}else if( _esWAKE(instruccion)){
		if(!hilo->kernelMode){
			abortar();
		}else{
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,"wake",5,0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,&hilo->B,sizeof(int32_t),0);
			t_list* params = list_create();
			char* nullable = crearString("NULL");
			list_add(params,nullable);
			printf(KRED);
			ejecucion_instruccion("WAKE",params);
			printf(KWHT);
			list_destroy_and_destroy_elements(params,(void*)destructorParams);
		}

	}else if( _esSHIF(instruccion)){
		char* algo = leerDeMemoria(hilo->pid,hilo->P,4);
		if(algo == NULL){
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,sefl,5,0);
			termino = 1;
		}else{
			int numero = (*((int *)algo));
			hilo->P = sumar(4,hilo->P);
			char* registro = leerDeMemoria(hilo->pid,hilo->P,1);
			if(registro == NULL){
				send(socketKernel,&soyCPU,sizeof(int),0);
				recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
				send(socketKernel,sefl,5,0);
				termino = 1;
			}else{
				hilo->P = sumar(1,hilo->P);
				int valor = obtenerValorDeMemoriaDe(registro, hilo);
				if(numero < 0){
					valor = valor << abs(numero);
				}else{
					valor = valor >> numero;
				}
				t_list* params = list_create();
				list_add(params,string_itoa(numero));
				list_add(params,registro);
				printf(KYEL);
				ejecucion_instruccion("SHIF",params);
				printf(KWHT);
				cargarNumeroEn(valor,registro, hilo);
				list_destroy_and_destroy_elements(params,(void*)destructorParams);
			}
		}

	}else if( _esNOPP(instruccion)){
		//no hace nada...
		t_list* params = list_create();
		char* nullable = crearString("NULL");
		list_add(params,nullable);
		printf(KYEL);
		ejecucion_instruccion("NOPP",params);
		printf(KWHT);
		list_destroy_and_destroy_elements(params,(void*)destructorParams);

	}else if( _esPUSH(instruccion)){
		char* algo = leerDeMemoria(hilo->pid,hilo->P,4);
		if(algo == NULL){
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,sefl,5,0);
			termino = 1;
		}else{
			int numero = (*((int *)algo));
			hilo->P = sumar(4,hilo->P);
			char* registro = leerDeMemoria(hilo->pid,hilo->P,1);
			if(registro == NULL){
				send(socketKernel,&soyCPU,sizeof(int),0);
				recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
				send(socketKernel,sefl,5,0);
				termino = 1;
			}else{
				hilo->P = sumar(1,hilo->P);
				//apila los primeros bytes (segun el numero) del registro al stack
				int valor = obtenerValorDeMemoriaDe(registro, hilo);
				int result;
				if(valor<0){
					char bytes[4];
					bytes[3] = (valor >> 24) & 0xFF;
					bytes[2] = (valor >> 16) & 0xFF;
					bytes[1] = (valor >> 8) & 0xFF;
					bytes[0] = valor & 0xFF;
					char* bytesnum = malloc(numero);
					if(numero >= 1){
						bytesnum[0] = bytes[0];
						if(numero >=2){
							bytesnum[1] = bytes[1];
							if(numero >=3){
								bytesnum[2] = bytes[2];
								if(numero == 4){
									bytesnum[3] = bytes[3];
								}else{
									bytesnum[3] = 0;
								}
							}else{
								bytesnum[2] = 0;
								bytesnum[3] = 0;
							}
						}else{
							bytesnum[1] = 0;
							bytesnum[2] = 0;
							bytesnum[3] = 0;
						}
					}
					result = escribirEnMemoria(bytesnum,hilo->pid,hilo->S,numero);
					free(bytesnum);
				}else{
					unsigned char* bytesnum = malloc(numero);
					bytesnum = conversorIntByteCode(numero, valor, bytesnum);
					result = escribirEnMemoria(bytesnum,hilo->pid,hilo->S,numero);
					free(bytesnum);
				}
				if (result == -1){
					send(socketKernel,&soyCPU,sizeof(int),0);
					recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
					send(socketKernel,sefe,5,0);
					termino = 1;
				}else{
					hilo->S = sumar(4,hilo->S);
				}
				t_list* params = list_create();
				list_add(params,string_itoa(numero));
				list_add(params,registro);
				printf(KYEL);
				ejecucion_instruccion("PUSH",params);
				printf(KWHT);
				list_destroy_and_destroy_elements(params,(void*)destructorParams);
			}
		}

	}else if( _esTAKE(instruccion)){
		char* algo = leerDeMemoria(hilo->pid,hilo->P,4);
		if(algo == NULL){
			send(socketKernel,&soyCPU,sizeof(int),0);
			recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
			send(socketKernel,sefl,5,0);
			termino = 1;
		}else{
			int numero = (*((int *)algo));
			hilo->P = sumar(4,hilo->P);
			char* registro = leerDeMemoria(hilo->pid,hilo->P,1);
			if(registro == NULL){
				send(socketKernel,&soyCPU,sizeof(int),0);
				recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
				send(socketKernel,sefl,5,0);
				termino = 1;
			}else{
				hilo->P = sumar(1,hilo->P);
				//desapila los primeros bytes (segun el numero) del stack al registro
				/*int valor = obtenerValorDeMemoriaDe(registro, hilo);*/
				char* contenido = leerDeMemoria(hilo->pid,hilo->S-4,4);
				if(contenido == NULL){
					send(socketKernel,&soyCPU,sizeof(int),0);
					recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
					send(socketKernel,sefl,5,0);
					termino = 1;
				}else{
					int numerito = (*(int*)contenido);
					hilo->S = restar(4,hilo->S);
					t_list* params = list_create();
					list_add(params,string_itoa(numero));
					list_add(params,registro);
					printf(KYEL);
					ejecucion_instruccion("TAKE",params);
					printf(KWHT);
					cargarNumeroEn(numerito,registro,hilo);
					list_destroy_and_destroy_elements(params,(void*)destructorParams);
				}
			}
		}

	}else if( _esXXXX(instruccion)){
		char* finDeEjec = crearString("xxxx");
		send(socketKernel,&soyCPU,sizeof(int),0);
		recv(socketKernel,&rpaKernel,sizeof(int),MSG_WAITALL);
		send(socketKernel,finDeEjec,strlen(finDeEjec)+1,0);
		free(finDeEjec);
		mandarTCBAlKernel(hilo);
		t_list* params = list_create();
		char* nullable = crearString("NULL");
		list_add(params,nullable);
		printf(KYEL);
		ejecucion_instruccion("XXXX",params);
		printf(KWHT);
		list_destroy_and_destroy_elements(params,(void*)destructorParams);

	}else{
		log_error(logs,"La instrucción leida es inválida");
		abortar();
	}
	cambiosRegistrosPanel(hilo);
	sleep(retardo);
	return hilo;
}
