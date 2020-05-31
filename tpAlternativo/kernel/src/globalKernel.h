#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <commons/log.h>
#include <commons/config.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/string.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "/home/utnso/git/ansisop-panel/panel/panel.c"
#include "/home/utnso/git/ansisop-panel/panel/kernel.c"

#define refKernelConfig "/home/utnso/git/tp-2014-2c-rafagadeterror/tpAlternativo/kernel/kernel.conf"
#define BACKLOG 5

fd_set conjuntoMaestro;
int socketCpuEspera;


enum {
	segFaultEliminar,
	problemaCPUHiloKernel,
	seInterrumpioCPU,
	segFaultLectura,
	creacionStack,
	abortoCPU,
	memoryOverload,
	divisionPorCero,
	finalizoHiloHijo,
	finalizoHiloPadre,
	segFaultEscritura
};

typedef struct {
	char* ipMSP;
	char* puertoMSP;
	char* puertoEscucha;
	int32_t quantum;
	int32_t tamanioStack;
	char* syscalls; //hay q ver que path es el correcto...
} thread_parm_t;

t_log* logs;
int socketCondicion;
int soyKernel = 0;
int contadorDePIDs;
int cantDeConsolas;
int numHilos = 0;
int socketMSP;



typedef struct {
	int tamCod;
	char* codigo;
} socketStruct;

typedef struct {
	int socketCPU;
	int32_t tidactual;
} t_cpu;

typedef struct {
	int socket;
	int pid;
} t_consola;

typedef struct {
	int32_t pid;
	u_int32_t tam_o_base;
}__attribute__ ((__packed__)) t_reservar_destruir; //esa anotación adelante es para que no asigne bytes de más

typedef struct {
	int32_t pid;
	u_int32_t dirLogica;
	int tam;
}__attribute__ ((__packed__)) t_leer;

typedef struct {
	int32_t pid;
	u_int32_t dirLogica;
	int tam;
	char* bytesAEscribir;
}__attribute__ ((__packed__)) t_escribir;

typedef struct {
	int longitud;
	char* dataAEnviar;
} t_stream2;

int buscador, buscadorCpu, buscadorPorPid, buscadorPorTID, BuscadorTCB;



int _is_consolaBuscada(t_consola* elem) {
	return elem->socket == buscador;
}

int _is_CPUBuscado(t_cpu* elem) {
	return elem->socketCPU == buscadorCpu;
}

int _esConsolaPorPid(t_consola* elem) {
	return elem->pid == buscadorPorPid;
}

int _esCPUPorTID(t_cpu* elem) {
	return elem->tidactual == buscadorPorTID;
}

void _mostrarConsola(t_consola* elem) {
	printf("Socket: %d\tPID: %d\n", elem->socket, elem->pid);
}

void _consolaDestroy(t_consola* elem) {
	free(elem);
}

void _cpuDestroy(t_cpu* elem) {
	free(elem);
}

int32_t devolvemeElValorEnteroDe(char* clave, t_config* config) {
	if (config_has_property(config, clave)) {
		int32_t valor = config_get_int_value(config, clave);
		return valor;
	} else {
		printf("No existe la propiedad %s", clave);
		log_error(logs, "No existe una propiedad");
		log_destroy(logs);
		exit(1);
	}
}

char* devolvemeElValorDe(char* clave, t_config* config) {
	if (config_has_property(config, clave)) {
		char* valor = config_get_string_value(config, clave);
		return valor;
	} else {
		printf("No existe la propiedad %s", clave);
		log_error(logs, "No existe una propiedad");
		log_destroy(logs);
		exit(1);
	}
}
bool _esNuevo(char* instruccion){
	return string_equals_ignore_case(instruccion, "newc");
}


bool _esEntradaN(char* instruccion){
	return string_equals_ignore_case(instruccion, "entn");
}

bool _esEntradaC(char* instruccion){
	return string_equals_ignore_case(instruccion, "entc");
}

bool _esXXXX(char* instruccion) {
	return string_equals_ignore_case(instruccion, "xxxx");
}

bool _esDQUA(char* instruccion) {
	return string_equals_ignore_case(instruccion, "dqua");
}

bool _esABOR(char* instruccion) {
	return string_equals_ignore_case(instruccion, "abor");
}

bool _esCREA(char* instruccion) {
	return string_equals_ignore_case(instruccion, "crea");
}

bool _esINTE(char* instruccion) {
	return string_equals_ignore_case(instruccion, "inte");
}

bool _esMALC(char* instruccion) {
	return string_equals_ignore_case(instruccion, "malc");
}

bool _esDIV0(char* instruccion) {
	return string_equals_ignore_case(instruccion, "div0");
}

bool _esSEFE(char* instruccion) {
	return string_equals_ignore_case(instruccion, "sefe");
}

bool _esSEFL(char* instruccion) {
	return string_equals_ignore_case(instruccion, "sefl");
}

bool _esFREE(char* instruccion) {
	return string_equals_ignore_case(instruccion, "free");
}

bool _esINNN(char* instruccion) {
	return string_equals_ignore_case(instruccion, "innn");
}

bool _esINNC(char* instruccion) {
	return string_equals_ignore_case(instruccion, "innc");
}

bool _esOUTN(char* instruccion) {
	return string_equals_ignore_case(instruccion, "outn");
}

bool _esOUTC(char* instruccion) {
	return string_equals_ignore_case(instruccion, "outc");
}

bool _esJOIN(char* instruccion) {
	return string_equals_ignore_case(instruccion, "join");
}

bool _esBLOK(char* instruccion) {
	return string_equals_ignore_case(instruccion, "blok");
}

bool _esWAKE(char* instruccion) {
	return string_equals_ignore_case(instruccion, "wake");
}

bool _esLCPU(char* instruccion) {
	return string_equals_ignore_case(instruccion, "lcpu");
}

bool _esFinQ(char* instruccion) {
	return string_equals_ignore_case(instruccion, "finq");
}

int conectarCliente(char *ip, char* port, t_log *logs) {
	int s;
	struct sockaddr_in dir;
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		log_error(logs, "No se pudo crear el socket");
		return -1;
	}
	dir.sin_family = AF_INET;
	dir.sin_port = htons(atoi(port));
	dir.sin_addr.s_addr = inet_addr(ip);
	if (connect(s, (struct sockaddr*) &dir, sizeof(dir)) == -1) {
		close(s);
		return -1;
	}
	return s;
}

int conectarMSP(char* ip, char* port, t_log *logs) {
	int socket_msp = conectarCliente(ip, port, logs);
	if (socket_msp < 0) {
		log_error(logs, "No se pudo conectar a la MSP correctamente");
		exit(0);
	} else
		log_info(logs, "Se conecto con la MSP correctamente");
	return socket_msp;
}

char* crearString(char* stringACrear) {
	char* tmp = malloc(sizeof(char) * strlen(stringACrear) + 1);
	memcpy(tmp, stringACrear, strlen(stringACrear));
	tmp[strlen(stringACrear)] = '\0';
	return tmp;
}

void avisarErrorAConsola(int opcion, int socketConsola) {
	int num = 5;
	send(socketConsola, &num, sizeof(int), 0);
	sleep(5);
	send(socketConsola, &opcion, sizeof(int), 0);
}
