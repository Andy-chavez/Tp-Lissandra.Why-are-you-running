/*
 * kernel_operaciones.h
 *
 *  Created on: 16 may. 2019
 *      Author: utnso
 */

#ifndef KERNEL_OPERACIONES_H_
#define KERNEL_OPERACIONES_H_
#include "kernel_configuraciones.h"
#include "kernel_structs-basicos.h"

/******************************DECLARACIONES******************************************/
void kernel_almacenar_en_cola(char*,char*);
void kernel_agregar_cola_proc_nuevos(char*);
void kernel_run(char*);
void kernel_consola();
int kernel_api(char*);
void guardarTablacreada(char*);
void eliminarTablaCreada(char* );
/* TODO implementaciones
 * los primeros 5 pasarlos a la memoria elegida por el criterio de la tabla
 * metrics -> variables globales con semaforos
 * journal -> pasarselo a memoria
 *
 */
void kernel_roundRobin();
int kernel_insert(char*);
int kernel_select(char*);
int kernel_describe(char*);
int kernel_create(char*);
int kernel_drop(char*);
int kernel_journal();
int kernel_metrics();
int kernel_add(char*);
/******************************IMPLEMENTACIONES******************************************/
//------ TABLAS ---------
void guardarTablaCreada(char* parametros){
	char** opAux =string_n_split(parametros,3," ");
	tabla* tablaAux = malloc(sizeof(tabla));
	tablaAux->nombreDeTabla= *opAux;
	if(string_equals_ignore_case(*(opAux+1),"SC")){
		tablaAux->consistenciaDeTabla = SC;
	}
	else if(string_equals_ignore_case(*(opAux+1),"SH")){
		tablaAux->consistenciaDeTabla = SH;
	}
	else if(string_equals_ignore_case(*(opAux+1),"EC")){
		tablaAux->consistenciaDeTabla = EC;
	}
	list_add(tablas,tablaAux);
}
void eliminarTablaCreada(char* parametros){
	tabla* tablaAux = malloc(sizeof(tabla));
	bool tablaDeNombre(tabla* t){
			return t->nombreDeTabla == parametros;
		}
	tablaAux = list_remove_by_condition(tablas, (void*)tablaDeNombre);
	free(tablaAux->nombreDeTabla);
	free(tablaAux);
}
tabla* encontrarTablaPorNombre(char* nombre){
	bool tablaDeNombre(tabla* t){
			return t->nombreDeTabla == nombre;
		}
	return list_find(tablas,(void* ) tablaDeNombre);
}
//------ MEMORIAS ---------
memoria* encontrarMemoria(int numero){
	bool memoriaEsNumero(memoria* mem) {
		return mem->numero == numero;
	}
	memoria * memory = malloc(sizeof(memoria));
	memory = (memoria*) list_find(conexionesMemoria, (void*)memoriaEsNumero);
	return memory;
}
memoria* encontrarMemoriaStrong(){
//	bool memoriaRandom(memoria* mem) {
//		return mem->numero == numero;
//	}
//
//	return (memoria*) list_find(criterios[criterio].memorias, (void*)memoriaRandom);   de momento sale hardcodeo de la unica memoria que hay

//	memoria* mem = malloc(sizeof(memoria));
//	mem->ip = ipMemoria;
//	mem->puerto = puertoMemoria;
//	mem->numero = numPrueba;
//	list_add(criterios[STRONG].memorias,mem);
	return list_get(criterios[STRONG].memorias, 0);
}
//------ CRITERIOS ---------


//------ CONEXION ---------
int encontrarSocketDeMemoria(int numero){
	bool encontrarSocket(memoria* unaConex){
		return unaConex->numero == numero;
	}
	memoria* mem = list_find(conexionesMemoria,(void*) encontrarSocket);
	return mem->socket;
}

int socketMemoriaSolicitada(consistencia criterio){
	memoria* mem = NULL;
	switch (criterio){

		case SC:
			mem = encontrarMemoriaStrong();
			break;
		case SH:

			break;
		case EC:
			break;
	}

	return encontrarSocketDeMemoria(mem->numero);
}
//------ ERRORES ---------
bool falloOperacionLQL(void* buffer){
	char* recibido = (char*) buffer;
	string_to_upper(recibido);
	return string_contains(recibido, "ERROR");
}
// _____________________________.: OPERACIONES DE API PARA LAS CUALES SELECCIONAR MEMORIA SEGUN CRITERIO:.____________________________________________
int kernel_insert(char* operacion){ //ya funciona, ver lo de seleccionar la memoria a la cual mandarle esto
	operacionLQL* opAux=splitear_operacion(operacion);
	int socket = socketMemoriaSolicitada(SC);
	serializarYEnviarOperacionLQL(socket, opAux);
	log_info(kernel_configYLog->log, "ENVIADO: %s", operacion);
	char* recibido = (char*) recibir(socket);
	if(falloOperacionLQL(recibir(socket))){
		log_error(kernel_configYLog->log, "RECIBIDO: %s", recibido);
		free(recibido);
		liberarOperacionLQL(opAux);
		return -1;
	}
	log_info(kernel_configYLog->log, "RECIBIDO: %s", recibido);
	free(recibido);
	liberarOperacionLQL(opAux);
	return 0;
}
int kernel_select(char* operacion){
	operacionLQL* opAux=splitear_operacion(operacion);
	int socket = socketMemoriaSolicitada(SC);
	serializarYEnviarOperacionLQL(socket, opAux);
	log_info(kernel_configYLog->log, "ENVIADO: %s", operacion);
	char* recibido = (char*) recibir(socket);
	if(falloOperacionLQL(recibir(socket))){
		log_error(kernel_configYLog->log, "RECIBIDO: %s", recibido);
		free(recibido);
		liberarOperacionLQL(opAux);
		return -1;
	}
	log_info(kernel_configYLog->log, "RECIBIDO: %s", recibido);
	free(recibido);
	liberarOperacionLQL(opAux);
	return 0;
}
int kernel_create(char* operacion){
	operacionLQL* opAux=splitear_operacion(operacion);
	guardarTablaCreada(opAux->parametros);
	int socket = socketMemoriaSolicitada(SC); //todo verificar lo de la tabla
	serializarYEnviarOperacionLQL(socket, opAux);
	char* recibido = (char*) recibir(socket);
	if(falloOperacionLQL(recibir(socket))){
		log_error(kernel_configYLog->log, "RECIBIDO: %s", recibido);
		free(recibido);
		liberarOperacionLQL(opAux);
		return -1;
	}
	log_info(kernel_configYLog->log, "RECIBIDO: %s", recibido);
	free(recibido);
	liberarOperacionLQL(opAux);
	return 0;
}
int kernel_describe(char* operacion){
	operacionLQL* opAux=splitear_operacion(operacion);
	int socket = socketMemoriaSolicitada(SC); //todo verificar lo de la tabla
	serializarYEnviarOperacionLQL(socket, opAux);
	log_info(kernel_configYLog->log, "ENVIADO: %s", operacion);
	void* recibirBuffer = recibir(socket);;
	metadata* met = deserializarMetadata(recibirBuffer);
	//TODO ACTUALIZAR ESTRUCTURAS
	log_info(kernel_configYLog->log, "RECIBIDO: %s %d", met->nombreTabla, met->tipoConsistencia);
	free(recibirBuffer);
	free(met->nombreTabla);
	free(met);
	//return 1;
	free(opAux->operacion);
	free(opAux->parametros);
	free(opAux);
	return 1;
}
int kernel_drop(char* operacion){
	operacionLQL* opAux=splitear_operacion(operacion);
	int socket = socketMemoriaSolicitada(SC); //todo verificar lo de la tabla
	serializarYEnviarOperacionLQL(socket, opAux);
	char* recibido = (char*) recibir(socket);
	if(falloOperacionLQL(recibir(socket))){
		log_error(kernel_configYLog->log, "RECIBIDO: %s", recibido);
		free(recibido);
		liberarOperacionLQL(opAux);
		return -1;
	}
	log_info(kernel_configYLog->log, "RECIBIDO: %s", recibido);
	free(recibido);
	liberarOperacionLQL(opAux);
	return 0;
}
// _____________________________.: OPERACIONES DE API DIRECTAS:.____________________________________________
int kernel_journal(){
	operacionLQL* opAux=splitear_operacion("JOURNAL");
	int socket = socketMemoriaSolicitada(SC); //todo verificar lo de la tabla
	serializarYEnviarOperacionLQL(socket, opAux);
	char* recibido = (char*) recibir(socket);
	if(falloOperacionLQL(recibir(socket))){
		log_error(kernel_configYLog->log, "RECIBIDO: %s", recibido);
		free(recibido);
		liberarOperacionLQL(opAux);
		return -1;
	}
	log_info(kernel_configYLog->log, "RECIBIDO: %s", recibido);
	free(recibido);
	liberarOperacionLQL(opAux);
	return 0;
}
int kernel_metrics(){
	printf("Not yet -> metrics\n");
	return 0;
}

int kernel_add(char* operacion){
	char** opAux = string_n_split(operacion,5," ");
	int numero = atoi(*(opAux+2));
	memoria* mem;
	if((mem = encontrarMemoria(numero))){
		if(string_equals_ignore_case(*(opAux+4),"HASH")){
			list_add(criterios[HASH].memorias, mem );
			kernel_journal();
			//todo journal cada vez que se agregue una aca a TODAS las memorias de este criterio
		}
		else if(string_equals_ignore_case(*(opAux+4),"STRONG")){
			list_add(criterios[STRONG].memorias, mem );
		}
		else if(string_equals_ignore_case(*(opAux+4),"EVENTUAL")){
			list_add(criterios[EVENTUAL].memorias, mem );
		}
		return 0;
	}
	else{
		log_error(kernel_configYLog->log,"EXEC: %s.Memmoria no conectada.", operacion);
		return -1;
	}
	liberarParametrosSpliteados(opAux);
}
// _________________________________________.: PROCEDIMIENTOS INTERNOS :.____________________________________________
bool instruccion_no_ejecutada(instruccion* instruc){
	return instruc->ejecutado==0;
}
// ---------------.: THREAD ROUND ROBIN :.---------------
void kernel_roundRobin(){
	while(1){
		sem_wait(&hayReady);
		pcb* pcb_auxiliar;
		pthread_mutex_lock(&colaListos);
		pcb_auxiliar = (pcb*) list_remove(cola_proc_listos,0);
		pthread_mutex_unlock(&colaListos);

		if(pcb_auxiliar->instruccion == NULL){
				pcb_auxiliar->ejecutado=1;
				if(kernel_api(pcb_auxiliar->operacion)==-1)
					log_error(kernel_configYLog->log,"EXEC: %s", pcb_auxiliar->operacion);
				pthread_mutex_lock(&colaTerminados);
				list_add(cola_proc_terminados,pcb_auxiliar);
				pthread_mutex_unlock(&colaTerminados);
				usleep(sleepEjecucion*1000);
				continue;
			}
		else if(pcb_auxiliar->instruccion !=NULL){
			int ERROR= 0;
			for(int quantum=0;quantum<quantumMax;quantum++){
				if(pcb_auxiliar->ejecutado ==0){
					pcb_auxiliar->ejecutado=1;
					if(kernel_api(pcb_auxiliar->operacion)==0){
						log_error(kernel_configYLog->log,"EXEC: %s", pcb_auxiliar->operacion);
						ERROR = -1;
						break;
					}
				}
				instruccion* instruc = list_find(pcb_auxiliar->instruccion,(void*)instruccion_no_ejecutada);
				if (instruc == NULL){
					break;
				}
				instruc->ejecutado = 1;
				if(kernel_api(instruc->operacion)==0){
					log_error(kernel_configYLog->log,"EXEC: %s", pcb_auxiliar->operacion);
					ERROR = -1;
					break;
				}
				usleep(sleepEjecucion*1000);
			}
			if(list_any_satisfy(pcb_auxiliar->instruccion,(void*)instruccion_no_ejecutada) && ERROR !=-1){
				pthread_mutex_lock(&colaListos);
				list_add(cola_proc_listos, pcb_auxiliar);
				pthread_mutex_unlock(&colaListos);
				sem_post(&hayReady);
			}
			else{
				pthread_mutex_lock(&colaTerminados);
				list_add(cola_proc_terminados, pcb_auxiliar);
				pthread_mutex_unlock(&colaTerminados);
				usleep(sleepEjecucion*1000);
				continue;
			}

		}
	}
}

// ---------------.: THREAD CONSOLA A NEW :.---------------
void kernel_almacenar_en_new(char*operacion){

	pthread_mutex_lock(&colaNuevos);
	list_add(cola_proc_nuevos, operacion);
	pthread_mutex_unlock(&colaNuevos);
	sem_post(&hayNew);
	pthread_mutex_lock(&mLog);
	log_info(kernel_configYLog->log, "NEW: %s", operacion);
	pthread_mutex_unlock(&mLog);
}

void kernel_consola(){
	printf("Proceso Kernel:	Ingrese la operacion que desea ejecutar y siga su ejecución mediante el archivo KERNEL.log\n");
	char* linea= NULL;
	while(1){
		linea = readline("");
		kernel_almacenar_en_new(linea);
	}
}
// ---------------.: THREAD NEW A READY :.---------------
void kernel_crearPCB(char* operacion){
	pcb* pcb_auxiliar = malloc(sizeof(pcb));
	pcb_auxiliar->operacion = operacion;
	pcb_auxiliar->ejecutado = 0;
	pcb_auxiliar->instruccion = NULL;
	pthread_mutex_lock(&colaNuevos);
	list_add(cola_proc_listos,pcb_auxiliar);
	pthread_mutex_unlock(&colaNuevos);
	sem_post(&hayReady);
}
void kernel_pasar_a_ready(){
	while(1){
		sem_wait(&hayNew);
		pthread_mutex_lock(&colaNuevos);
		char* operacion = NULL;
		operacion =(char*) list_remove(cola_proc_nuevos,0);
		pthread_mutex_unlock(&colaNuevos);

		string_to_upper(operacion);
		if (string_contains( operacion, "RUN")) {
			kernel_run(operacion);
		}
		else if(string_contains(operacion, "SELECT") || string_contains(operacion, "INSERT") ||
				string_contains(operacion, "CREATE") || string_contains(operacion, "DESCRIBE") ||
				string_contains(operacion, "DROP") ||  string_contains(operacion, "JOURNAL") ||
				string_contains(operacion, "METRICS") || string_contains(operacion, "ADD")){
			kernel_crearPCB(operacion);
		}
		else{
			log_error(kernel_configYLog->log,"NEW: %s", operacion);
		}
	}
}
void kernel_run(char* operacion){
	char** opYArg;
	opYArg = string_n_split(operacion ,2," ");
	string_to_lower(*(opYArg+1));
	FILE *archivoALeer;
	if ((archivoALeer= fopen((*(opYArg+1)), "r")) == NULL){
		log_error(kernel_configYLog->log,"EXEC: %s %s. (Consejo: verifique existencia del archivo)", *opYArg, *(opYArg+1) ); //operacion);
		free(*(opYArg+1));
		free(*(opYArg));
		free(opYArg);
		exit(EXIT_FAILURE);
	}
	char *lineaLeida;
	size_t limite = 250;
	ssize_t leer;
	lineaLeida = NULL;
	pcb* pcb_auxiliar = malloc(sizeof(pcb));
	pcb_auxiliar->operacion = operacion;
	pcb_auxiliar->ejecutado = 1 ;
	pcb_auxiliar->instruccion =list_create();

	while((leer = getline(&lineaLeida, &limite, archivoALeer)) != -1){
		instruccion* instruccion_auxiliar = malloc(sizeof(instruccion));
		instruccion_auxiliar->ejecutado= 0;
		if(*(lineaLeida + leer - 1) == '\n') {
			*(lineaLeida + leer - 1) = '\0';
		}
		instruccion_auxiliar->operacion= string_duplicate(lineaLeida);
		list_add(pcb_auxiliar->instruccion,instruccion_auxiliar);
	}
	pthread_mutex_lock(&colaNuevos);
	list_add(cola_proc_listos,pcb_auxiliar);
	pthread_mutex_unlock(&colaNuevos);
	free(*(opYArg+1));
	free(*(opYArg));
	free(opYArg);
	fclose(archivoALeer);
	sem_post(&hayReady);
}
int kernel_api(char* operacionAParsear)
{
	if(string_contains(operacionAParsear, "INSERT")) {
		return kernel_insert(operacionAParsear);
	}
	else if (string_contains(operacionAParsear, "SELECT")) {
		return kernel_select(operacionAParsear);
	}
	else if (string_contains(operacionAParsear, "DESCRIBE")) {
		return kernel_describe(operacionAParsear);
	}
	else if (string_contains(operacionAParsear, "CREATE")) {
		return kernel_create(operacionAParsear);
	}
	else if (string_contains(operacionAParsear, "DROP")) {
		return kernel_drop(operacionAParsear);
	}
	else if (string_contains(operacionAParsear, "ADD")){
		return kernel_add(operacionAParsear);
	}
	else if (string_contains(operacionAParsear, "JOURNAL")) {
		free(operacionAParsear);
		return kernel_journal();
	}
	else if (string_contains(operacionAParsear, "METRICS")) {
		free(operacionAParsear);
		return kernel_metrics();
	}
	else {
		log_error(kernel_configYLog->log,"EXEC: %s", operacionAParsear );
		free(operacionAParsear);
		return 0;
	}
}
#endif /* KERNEL_OPERACIONES_H_ */
