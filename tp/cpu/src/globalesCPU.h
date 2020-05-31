#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <string.h>
#include <commons/log.h>
#include <sys/socket.h>
#include <commons/config.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>

#define KGRN  "\x1B[32m"
#define KNRM  "\x1B[0m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define refcpuConfig "/home/utnso/git/tp-2014-2c-rafagadeterror/tp/cpu/cpu.conf"

t_log* logs;

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

typedef struct{
	int32_t pid;
	u_int32_t tam_o_base;
}__attribute__ ((__packed__)) t_reservar_destruir; //esa anotación adelante es para que no asigne bytes de más

typedef struct{
	int32_t pid;
	u_int32_t dirLogica;
	int tam;
}__attribute__ ((__packed__)) t_leer;

typedef struct{
	int32_t pid;
	u_int32_t dirLogica;
	int tam;
	char* bytesAEscribir;
}__attribute__ ((__packed__)) t_escribir;

typedef struct{
	int longitud;
	char* dataAEnviar;
}t_stream2;

typedef struct{
	char* ipKernel;
	char* ipMSP;
	char* puertoKernel;
	char* puertoMSP;
	int32_t retardo;
}thread_parm_t;

int32_t devolvemeElValorEnteroDe(char* clave,t_config* config){
	if( config_has_property(config,clave) ){
		int32_t valor = config_get_int_value(config,clave);
		return valor;
	} else {
		printf("No existe la propiedad %s",clave);
		log_error(logs,"No existe una propiedad");
		log_destroy(logs);
		exit(1);
	}
}

int32_t retardo;

char* devolvemeElValorDe(char* clave,t_config* config){
	if( config_has_property(config,clave) ){
		char* valor = config_get_string_value(config,clave);
		return valor;
	} else {
		printf("No existe la propiedad %s",clave);
		log_error(logs,"No existe una propiedad");
		log_destroy(logs);
		exit(1);
	}
}

int recibir(int socket, int size, void** dato){
	*dato = malloc(size);
	if (recv (socket, dato, size, MSG_WAITALL) <= 0){
		return 0;
	}
	return 1;
}

int conectarCliente(char *ip, char* port,t_log *logs){
	int s;
	struct sockaddr_in dir;
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0){
		log_error(logs, "No se pudo crear el socket");
		return -1;
	}
	dir.sin_family = AF_INET;
	dir.sin_port = htons(atoi(port));
	dir.sin_addr.s_addr = inet_addr(ip);
	if (connect(s, (struct sockaddr*) &dir, sizeof(dir))== -1) {
		close(s);
		return -1;
	}
	return s;
}

int conectarMSP(char* ip,char* port,t_log *logs){
	int socket_msp = conectarCliente(ip, port, logs);
	if(socket_msp < 0 ){
		log_error(logs, "No se pudo conectar a la MSP correctamente");
		exit(0);
	}else
		log_info(logs, "Se conecto con la MSP correctamente");
	return socket_msp;
}

int conectarKernel(char* ip,char* port,t_log *logs){
	int socket_kernel;
	socket_kernel = conectarCliente(ip, port,logs);
	if(socket_kernel < 0 ){
		log_error(logs, "No se pudo conectar al kernel correctamente");
		exit(0);
	}else
		log_info(logs, "Se conectó con el kernel correctamente");
	return socket_kernel;
}

t_stream* serializadorTcb(tcb* hilo){
	char* data= malloc(sizeof(int32_t)*10 + sizeof(u_int32_t)*5 + sizeof(int) * 2 );
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
	printf("Imprimiendo TCB:\n");
	printf("PID:%d\n",hilo->pid);
	printf("KM:%d\n",hilo->kernelMode);
	printf("TID:%d\n",hilo->tid);
	return hilo;
}

int socketKernel, socketMSP;
t_log* logs;
int soyCPU = 1;

char* crearString(char* stringACrear){
	char* tmp = malloc(sizeof(char)*strlen(stringACrear)+1);
	memcpy(tmp, stringACrear,strlen(stringACrear));
	tmp[strlen(stringACrear)] = '\0';
	return tmp;
}

int32_t sumar(int cantidad, int32_t cursor){
	cursor += cantidad;
	return cursor;
}

int32_t restar(int cantidad, int32_t cursor){
	cursor -= cantidad;
	return cursor;
}

bool _esLOAD(char* instruccion){
	return string_equals_ignore_case( instruccion, "load");
}

bool _esGETM(char* instruccion){
	return string_equals_ignore_case( instruccion, "getm");
}

bool _esSETM(char* instruccion){
	return string_equals_ignore_case( instruccion, "setm");
}

bool _esMOVR(char* instruccion){
	return string_equals_ignore_case( instruccion, "movr");
}

bool _esADDR(char* instruccion){
	return string_equals_ignore_case( instruccion, "addr");
}

bool _esSUBR(char* instruccion){
	return string_equals_ignore_case( instruccion, "subr");
}

bool _esMULR(char* instruccion){
	return string_equals_ignore_case( instruccion, "mulr");
}

bool _esMODR(char* instruccion){
	return string_equals_ignore_case( instruccion, "modr");
}

bool _esDIVR(char* instruccion){
	return string_equals_ignore_case( instruccion, "divr");
}

bool _esINCR(char* instruccion){
	return string_equals_ignore_case( instruccion, "incr");
}

bool _esDECR(char* instruccion){
	return string_equals_ignore_case( instruccion, "decr");
}

bool _esCOMP(char* instruccion){
	return string_equals_ignore_case( instruccion, "comp");
}

bool _esCGEQ(char* instruccion){
	return string_equals_ignore_case( instruccion, "cgeq");
}

bool _esCLEQ(char* instruccion){
	return string_equals_ignore_case( instruccion, "cleq");
}

bool _esGOTO(char* instruccion){
	return string_equals_ignore_case( instruccion, "goto");
}

bool _esJMPZ(char* instruccion){
	return string_equals_ignore_case( instruccion, "jmpz");
}

bool _esJPNZ(char* instruccion){
	return string_equals_ignore_case( instruccion, "jpnz");
}

bool _esINTE(char* instruccion){
	return string_equals_ignore_case( instruccion, "inte");
}

bool _esSHIF(char* instruccion){
	return string_equals_ignore_case( instruccion, "shif");
}

bool _esNOPP(char* instruccion){
	return string_equals_ignore_case( instruccion, "nopp");
}

bool _esPUSH(char* instruccion){
	return string_equals_ignore_case( instruccion, "push");
}

bool _esTAKE(char* instruccion){
	return string_equals_ignore_case( instruccion, "take");
}

bool _esXXXX(char* instruccion){
	return string_equals_ignore_case( instruccion, "xxxx");
}

bool _esMALC(char* instruccion){
	return string_equals_ignore_case( instruccion, "malc");
}

bool _esFREE(char* instruccion){
	return string_equals_ignore_case( instruccion, "free");
}

bool _esINNN(char* instruccion){
	return string_equals_ignore_case( instruccion, "innn");
}

bool _esINNC(char* instruccion){
	return string_equals_ignore_case( instruccion, "innc");
}

bool _esOUTN(char* instruccion){
	return string_equals_ignore_case( instruccion, "outn");
}

bool _esOUTC(char* instruccion){
	return string_equals_ignore_case( instruccion, "outc");
}

bool _esCREA(char* instruccion){
	return string_equals_ignore_case( instruccion, "crea");
}

bool _esJOIN(char* instruccion){
	return string_equals_ignore_case( instruccion, "join");
}

bool _esBLOK(char* instruccion){
	return string_equals_ignore_case( instruccion, "blok");
}

bool _esWAKE(char* instruccion){
	return string_equals_ignore_case( instruccion, "wake");
}

void mandarTCBAlKernel(tcb* hilo){
	int respuesta;
	recibir(socketKernel,sizeof(int),(void*)&respuesta);
	if(respuesta == 1){
		t_stream* stream = serializadorTcb(hilo);
		send(socketKernel,stream->datos, sizeof(tcb),0);
	}else{
		log_error(logs,"El kernel recibió erroneamente o no recibió la instrucción de fin de ciclo");
	}
}
