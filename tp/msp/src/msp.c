#include "atencionKernelYCPU.h"

int main(void) {
	logs = log_create("logMSP", "msp.c", 1, LOG_LEVEL_TRACE); //LOG
	parm = NULL;
	parm = malloc(sizeof(thread_parm_t));
	config = config_create(refmspConfig);

	parm->puertoEscucha = devolvemeElValorDe("PUERTOESCUCHA",config);
	parm->cantidadSwapping = devolvemeElValorEnteroDe("CANTIDADSWAPPING",config);
	parm->cantidadMemoria = devolvemeElValorEnteroDe("CANTIDADMEMORIA",config);
	parm->algoritmo = devolvemeElValorDe("ALGORITMO",config);
	parm->tamanioPagina = devolvemeElValorEnteroDe("TAMANIOPAGINA",config);
	tamanioDePagina= parm->tamanioPagina;

	parm->cantidadMemoria = parm->cantidadMemoria * 1024;
	parm->cantidadSwapping = parm->cantidadSwapping * 1024 * 1024;

	memMaxima = parm->cantidadMemoria;
	memReservada = 0;

	if(!_esLRU(parm->algoritmo) && !_esClock(parm->algoritmo)){
		log_error(logs,"El algoritmo %s no es valido",parm->algoritmo);
		exit(0);
	}else{
		int tam;
		tam = strlen(parm->algoritmo);
		algoritmo = malloc(sizeof(char)*tam);
		algoritmo = parm->algoritmo;
	}

	printf(KBLU "\nLa configuración del MSP es:\n");
	printf("PuertoEscucha:%s\n",parm->puertoEscucha);
	log_info(logs,"CantidadMemoria:%d\tCantidadSwapping:%d\n",parm->cantidadMemoria,parm->cantidadSwapping);
	printf("Algoritmo:%s\n", parm->algoritmo);
	printf("TamanioPagina:%d\n", parm->tamanioPagina);

	cantidadMaximaDePaginas = parm->cantidadMemoria/parm->tamanioPagina;
	cantMaxDePaginasSwapeadas = parm->cantidadSwapping/parm->tamanioPagina;
	cantidadArchivosSwapeados = 0;//para no pasarnos del espacio de intercambio maximo

	printf("Cantidad Maxima de Paginas: %d\n",cantidadMaximaDePaginas);
	printf("Cantidad Maxima de Paginas Swappeadas: %d\n",cantMaxDePaginasSwapeadas);
	printf("Cantidad Archivos en Disco: %d\n" KWHT,cantidadArchivosSwapeados);

	pthread_t consola;

	memoriaPrincipal = malloc(parm->cantidadMemoria);
	cantidadDeMarcos = cantidadMaximaDePaginas;

	inicializarEstructuras();//lista de segmentos, de marcos e inicializamos marcos.

	pthread_create(&consola,NULL,hiloConsola,NULL); //crea hilo consola
	atencionKernelYCPU(parm);
	pthread_join(consola,NULL);
	finalizar();
	return EXIT_SUCCESS;
}
