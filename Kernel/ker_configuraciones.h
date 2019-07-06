#ifndef KER_CONFIGURACIONES_H_
#define KER_CONFIGURACIONES_H_

#include <sys/types.h>
#include <sys/inotify.h>
#include "ker_auxiliares.h"
#include <stdio.h>
#include <stdlib.h>

#define EVENT_SIZE_CONFIG (sizeof(struct inotify_event) + 15)
#define BUF_LEN_CONFIG (1 * EVENT_SIZE_CONFIG)
/******************************DECLARACIONES******************************************/
void kernel_inicializarSemaforos();
void kernel_crearListas();
void liberarConfigYLogs();
void kernel_inicializarVariablesYListas();
void kernel_finalizar();
int kernel_inicializarMemoria();
/******************************IMPLEMENTACIONES******************************************/
// _________________________________________.: LLENAR/VACIAR VARIABLES GLOBALES :.____________________________________________
//-----------------RESETEAR VARIABLES-----------------------------
void memoria_resetearMetricas(memoria* mem){
		mem->cantidadIns = 0;
		mem->cantidadSel = 0;
}
void metrics_resetVariables(){
	pthread_mutex_lock(&mHash);
	criterios[HASH].cantidadInserts = 0;
	criterios[HASH].cantidadSelects = 0;
	criterios[HASH].tiempoInserts = 0;
	criterios[HASH].tiempoSelects = 0;
	list_iterate(criterios[HASH].memorias, (void*) memoria_resetearMetricas);
	pthread_mutex_unlock(&mHash);
	pthread_mutex_lock(&mStrong);
	criterios[STRONG].cantidadInserts = 0;
	criterios[STRONG].cantidadSelects = 0;
	criterios[STRONG].tiempoInserts = 0;
	criterios[STRONG].tiempoSelects = 0;
	list_iterate(criterios[STRONG].memorias, (void*) memoria_resetearMetricas);
	pthread_mutex_unlock(&mStrong);
	pthread_mutex_lock(&mEventual);
	criterios[EVENTUAL].cantidadInserts = 0;
	criterios[EVENTUAL].cantidadSelects = 0;
	criterios[EVENTUAL].tiempoInserts = 0;
	criterios[EVENTUAL].tiempoSelects = 0;
	list_iterate(criterios[EVENTUAL].memorias, (void*) memoria_resetearMetricas);
	pthread_mutex_unlock(&mEventual);

}
//-----------------INICIALIZAR KERNEL-----------------------------
void kernel_inicializarSemaforos(){
	pthread_mutex_init(&colaNuevos, NULL);
	pthread_mutex_init(&colaListos, NULL);
	pthread_mutex_init(&colaTerminados, NULL);
	pthread_mutex_init(&mLogMetrics, NULL);
	pthread_mutex_init(&mLog, NULL);
	pthread_mutex_init(&mThread, NULL);
	pthread_mutex_init(&mMemorias,NULL);
	pthread_mutex_init(&quantum, NULL);
	pthread_mutex_init(&sleepExec,NULL);
	pthread_mutex_init(&mMetadataRefresh,NULL);
	pthread_mutex_init(&mEventual,NULL);
	pthread_mutex_init(&mStrong,NULL);
	pthread_mutex_init(&mHash,NULL);
	pthread_mutex_init(&mConexion,NULL);
	sem_init(&hayNew,0,0);
	sem_init(&hayReady,0,0);
	sem_init(&finalizar,0,0);
	sem_init(&modificables,0,0);
}
void kernel_crearListas(){
	cola_proc_nuevos = list_create();
	cola_proc_listos = list_create();
	cola_proc_terminados = list_create();
	rrThreads = list_create();
	criterios[HASH].unCriterio = SHC;
	criterios[HASH].tiempoInserts = 0;
	criterios[HASH].tiempoSelects = 0;
	criterios[HASH].cantidadInserts = 0;
	criterios[HASH].cantidadSelects = 0;
	criterios[HASH].memorias = list_create();
	criterios[STRONG].unCriterio = SC;
	criterios[STRONG].tiempoInserts = 0;
	criterios[STRONG].tiempoSelects = 0;
	criterios[STRONG].cantidadInserts = 0;
	criterios[STRONG].cantidadSelects = 0;
	criterios[STRONG].memorias = list_create();
	criterios[EVENTUAL].unCriterio = EC;
	criterios[EVENTUAL].tiempoInserts = 0;
	criterios[EVENTUAL].tiempoSelects = 0;
	criterios[EVENTUAL].cantidadInserts = 0;
	criterios[EVENTUAL].cantidadSelects = 0;
	criterios[EVENTUAL].memorias = list_create();
	tablas = list_create();
	memorias = list_create();
}
int kernel_inicializarMemoria(){
	pthread_mutex_lock(&mConexion);
	int socketClienteKernel = crearSocketCliente(ipMemoria,puertoMemoria);
	pthread_mutex_unlock(&mConexion);
	if(socketClienteKernel==-1){
		return -1;
	}
	operacionProtocolo protocoloGossip = TABLAGOSSIP;
	enviar(socketClienteKernel,(void*)&protocoloGossip, sizeof(operacionProtocolo));
	recibirYDeserializarTablaDeGossipRealizando(socketClienteKernel,guardarMemorias);
	return 0;
}
void kernel_inicializarVariablesYListas(){
	logMetrics = log_create("Metrics.log", "KERNEL", 0, LOG_LEVEL_INFO);
	kernel_configYLog= malloc(sizeof(configYLogs));
	kernel_configYLog->config = config_create(pathConfig);
	kernel_configYLog->log = log_create("Kernel.log", "KERNEL", 0, LOG_LEVEL_INFO);
	ipMemoria = config_get_string_value(kernel_configYLog->config ,"IP_MEMORIA");
	puertoMemoria = config_get_string_value(kernel_configYLog->config,"PUERTO_MEMORIA");
	multiprocesamiento =config_get_int_value(kernel_configYLog->config,"MULTIPROCESAMIENTO");
	quantumMax = config_get_int_value(kernel_configYLog->config,"QUANTUM");
	metadataRefresh = config_get_int_value(kernel_configYLog->config,"METADATA_REFRESH");
	sleepEjecucion = config_get_int_value(kernel_configYLog->config,"SLEEP_EJECUCION");
	timedGossip = config_get_int_value(kernel_configYLog->config,"TIMED_GOSSIP");
	kernel_crearListas();
}
void kernel_inicializarEstructuras(){
	kernel_inicializarSemaforos();
}
//-----------------FINALIZAR KERNEL-----------------------------
void liberarConfigYLogs() {
	log_destroy(logMetrics);
	log_destroy(kernel_configYLog->log);
	config_destroy(kernel_configYLog->config);
	free(kernel_configYLog);
}
void destruirSemaforos(){
	sem_destroy(&hayNew);
	sem_destroy(&finalizar);
	sem_destroy(&hayReady);
	sem_destroy(&modificables);
	pthread_mutex_destroy(&colaNuevos);
	pthread_mutex_destroy(&colaListos);
	pthread_mutex_destroy(&colaTerminados);
	pthread_mutex_destroy(&mEventual);
	pthread_mutex_destroy(&mHash);
	pthread_mutex_destroy(&mStrong);
	pthread_mutex_destroy(&mLog);
	pthread_mutex_destroy(&mMemorias);
	pthread_mutex_destroy(&quantum);
	pthread_mutex_destroy(&sleepExec);
	pthread_mutex_destroy(&mMetadataRefresh);
	pthread_mutex_destroy(&mConexion);
	pthread_mutex_destroy(&mLogMetrics);
}
void liberarColas(pcb* element){
	free(element->operacion);
	free(element);
}
void liberarPCB(pcb* elemento) {
	void liberarInstrucciones(instruccion* instrucc) {
		free(instrucc->operacion);
		free(instrucc);
	}
	list_destroy_and_destroy_elements(elemento->instruccion,(void*) liberarInstrucciones);
	free(elemento->operacion);
	free(elemento);
}
void liberarMemoria(memoria* elemento) {
	free(elemento->ip);
	free(elemento->puerto);
	free(elemento);
}
void liberarTabla(tabla* t) {
	free(t->nombreDeTabla);
	free(t);
}
void liberarListas(){ //todo agregar RR
	pthread_mutex_lock(&colaNuevos);
	list_destroy_and_destroy_elements(cola_proc_nuevos,free);
	pthread_mutex_unlock(&colaNuevos);

	pthread_mutex_lock(&colaListos);
	list_destroy_and_destroy_elements(cola_proc_listos,(void*) liberarPCB);
	pthread_mutex_unlock(&colaListos);

	pthread_mutex_lock(&colaTerminados);
	list_destroy_and_destroy_elements(cola_proc_terminados,(void*) liberarPCB);
	pthread_mutex_unlock(&colaTerminados);

	pthread_mutex_lock(&mMemorias);
	list_destroy_and_destroy_elements(memorias,(void*)liberarMemoria);
	pthread_mutex_unlock(&mMemorias);

	pthread_mutex_lock(&mHash);
	list_destroy_and_destroy_elements(criterios[HASH].memorias,(void*)liberarMemoria);
	pthread_mutex_unlock(&mHash);

	pthread_mutex_lock(&mStrong);
	list_destroy_and_destroy_elements(criterios[STRONG].memorias,(void*)liberarMemoria);
	pthread_mutex_unlock(&mStrong);

	pthread_mutex_lock(&mEventual);
	list_destroy_and_destroy_elements(criterios[EVENTUAL].memorias,(void*)liberarMemoria);
	pthread_mutex_unlock(&mEventual);

	pthread_mutex_lock(&mTablas);
	list_destroy_and_destroy_elements(tablas,(void*)liberarTabla);
	pthread_mutex_unlock(&mTablas);
}
void kernel_finalizar(){
	liberarConfigYLogs();
	liberarListas();
	destruirSemaforos();
}
//----------------- CAMBIOS EN EJECUCION -----------------------------
void* cambiosConfig(){
	char buffer[BUF_LEN_CONFIG];
	int fdConfig = inotify_init();
	char* path = pathConfig;

 	if(fdConfig < 0) {
		pthread_mutex_lock(&mLog);
		log_error(kernel_configYLog->log,"Hubo un error con el inotify_init");
		pthread_mutex_unlock(&mLog);
	}

 	inotify_add_watch(fdConfig, path, IN_MODIFY);

 	while(1) {
		int size = read(fdConfig, buffer, BUF_LEN_CONFIG);

 		if(size<0) {
			pthread_mutex_lock(&mLog);
			log_error(kernel_configYLog->log,"Hubo un error al leer modificaciones del config");
			pthread_mutex_unlock(&mLog);
		}

 		t_config* configConNuevosDatos = config_create(path);

 		if(!configConNuevosDatos) {
			pthread_mutex_lock(&mLog);
			log_error(kernel_configYLog->log,"Hubo un error al abrir el archivo de config");
			pthread_mutex_unlock(&mLog);
		}

 		int desplazamiento = 0;

 		while(desplazamiento < size) {
			struct inotify_event *event = (struct inotify_event *) &buffer[desplazamiento];

 			if (event->mask & IN_MODIFY) {
 				pthread_mutex_lock(&mLog);
				log_info(kernel_configYLog->log,"Hubieron cambios en el archivo de config. Analizando y realizando cambios a retardos...");
				pthread_mutex_unlock(&mLog);

				pthread_mutex_lock(&quantum);
 				quantumMax = config_get_int_value(configConNuevosDatos, "RETARDO_GOSSIPING");
 				pthread_mutex_unlock(&quantum);
				pthread_mutex_lock(&sleepExec);
 				sleepEjecucion = config_get_int_value(configConNuevosDatos, "SLEEP_EJECUCION");
 				pthread_mutex_unlock(&sleepExec);
				pthread_mutex_lock(&mMetadataRefresh);
				metadataRefresh = config_get_int_value(configConNuevosDatos, "METADATA_REFRESH");
 				pthread_mutex_unlock(&mMetadataRefresh);
			}
 			config_destroy(configConNuevosDatos);
			desplazamiento += sizeof (struct inotify_event) + event->len;
		}
	}
}

#endif /* KER_CONFIGURACIONES_H_ */
