#include "serializacion.h"
#include <sys/socket.h>
#include <sys/types.h>

operacionProtocolo empezarDeserializacion(void **buffer) {
	operacionProtocolo protocolo;
	if(*buffer == NULL) {
		return DESCONEXION;
	}
	memcpy(&protocolo, *buffer, sizeof(operacionProtocolo));
	return protocolo;
}

void liberarOperacionLQL(operacionLQL* operacion) {
	free(operacion->operacion);
	free(operacion->parametros);
	free(operacion);
}

void* serializarHandshake(int tamanioValue, int* tamanioBuffer){
	int desplazamiento = 0;
	operacionProtocolo protocoloHandshake = HANDSHAKE;
	void *buffer= malloc((sizeof(int))*2);

	memcpy(buffer + desplazamiento, &protocoloHandshake, sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, &tamanioValue, sizeof(int));
	desplazamiento += sizeof(int);

	*(tamanioBuffer) = desplazamiento;

	return buffer;
}

int deserializarHandshake(void* bufferHandshake){
	int desplazamiento = 4;
	int tamanioDelValue;

	memcpy(&tamanioDelValue, bufferHandshake + desplazamiento, sizeof(int));

	free(bufferHandshake);
	return tamanioDelValue;
}

void serializarYEnviarHandshake(int socket, int tamanioValue) {
	int tamanioBuffer;
	void* bufferAEnviar = serializarHandshake(tamanioValue, &tamanioBuffer);
	enviar(socket, bufferAEnviar, tamanioBuffer);
	free(bufferAEnviar);
}

registroConNombreTabla* deserializarRegistro(void* bufferRegistro) {
	int desplazamiento = 4;
	registroConNombreTabla* unRegistro = malloc(sizeof(registroConNombreTabla));
	int largoDeNombreTabla, tamanioTimestamp, tamanioKey, largoDeValue;

	memcpy(&largoDeNombreTabla, bufferRegistro + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);
	unRegistro->nombreTabla = malloc(largoDeNombreTabla);

	memcpy(unRegistro->nombreTabla, bufferRegistro + desplazamiento, largoDeNombreTabla);
	desplazamiento+= largoDeNombreTabla;

	memcpy(&tamanioTimestamp, bufferRegistro + desplazamiento, sizeof(int));
	desplazamiento+= sizeof(int);

	memcpy( &(unRegistro->timestamp), bufferRegistro + desplazamiento, tamanioTimestamp);
	desplazamiento+= tamanioTimestamp;

	memcpy(&tamanioKey, bufferRegistro + desplazamiento , sizeof(int));
	desplazamiento+= sizeof(int);

	memcpy(&(unRegistro->key), bufferRegistro + desplazamiento, tamanioKey);
	desplazamiento+= tamanioKey;

	memcpy(&largoDeValue, desplazamiento + bufferRegistro, sizeof(int));
	desplazamiento+= sizeof(int);
	unRegistro->value = malloc(largoDeValue);

	memcpy(unRegistro->value, bufferRegistro + desplazamiento, largoDeValue);

	free(bufferRegistro);

	return unRegistro;
}

void* serializarUnRegistro(registroConNombreTabla* unRegistro, int* tamanioBuffer) {

	int desplazamiento = 0;
	int largoDeNombreTabla = strlen(unRegistro->nombreTabla) + 1;
	int largoDeValue = strlen(unRegistro->value) + 1;
	operacionProtocolo protocoloRegistro = UNREGISTRO;
	int tamanioKey = sizeof(u_int16_t);
	int tamanioTimeStamp = sizeof(time_t);
	int tamanioTotalBuffer = 5*sizeof(int) + largoDeNombreTabla + largoDeValue + tamanioKey + tamanioTimeStamp;
	void *bufferRegistro= malloc(tamanioTotalBuffer);

	//protocolo
	memcpy(bufferRegistro + desplazamiento, &protocoloRegistro, sizeof(int));
	desplazamiento+= sizeof(int);
	//Tamaño de nombre de tabla
	memcpy(bufferRegistro + desplazamiento, &largoDeNombreTabla, sizeof(int));
	desplazamiento+= sizeof(int);
	//Nombre de tabla
	memcpy(bufferRegistro + desplazamiento, unRegistro->nombreTabla, largoDeNombreTabla);
	desplazamiento+= sizeof(char)*largoDeNombreTabla;
	//Tamaño de timestamp
	memcpy(bufferRegistro + desplazamiento, &tamanioTimeStamp, sizeof(int));
	desplazamiento+= sizeof(int);
	//Nombre de timestamp
	memcpy(bufferRegistro + desplazamiento, &(unRegistro->timestamp), tamanioTimeStamp);
	desplazamiento+= tamanioTimeStamp;
	//Tamaño de key
	memcpy(bufferRegistro + desplazamiento, &tamanioKey, sizeof(int));
	desplazamiento+= sizeof(int);
	//Nombre de key
	memcpy(bufferRegistro + desplazamiento, &(unRegistro->key), sizeof(u_int16_t));
	desplazamiento+= tamanioKey;
	//Tamaño de value
	memcpy(bufferRegistro + desplazamiento, &largoDeValue, sizeof(int));
	desplazamiento+= sizeof(int);
	//Nombre de value
	memcpy(bufferRegistro + desplazamiento, unRegistro->value, largoDeValue);
	desplazamiento+= largoDeValue;

	*(tamanioBuffer) = desplazamiento;

	return bufferRegistro;
}

void serializarYEnviarRegistro(int socket, registroConNombreTabla* unRegistro) {
	int tamanioAEnviar;
	void* bufferAEnviar = serializarUnRegistro(unRegistro, &tamanioAEnviar);
	enviar(socket, bufferAEnviar, tamanioAEnviar);
	free(bufferAEnviar);
}

operacionLQL* _deserializarOperacionSinFree(void* bufferOperacion, int* tamanioTotal) {
	int desplazamiento = 4;
	int tamanioOperacion,largoDeParametros;
	operacionLQL* unaOperacion = malloc(sizeof(operacionLQL));

	memcpy(&tamanioOperacion,bufferOperacion + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);
	unaOperacion->operacion = malloc(tamanioOperacion);

	memcpy((unaOperacion->operacion),bufferOperacion + desplazamiento, tamanioOperacion);
	desplazamiento += tamanioOperacion;

	memcpy(&largoDeParametros ,bufferOperacion + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);
	unaOperacion->parametros = malloc(largoDeParametros);

	memcpy(unaOperacion->parametros,bufferOperacion + desplazamiento, largoDeParametros);
	desplazamiento += largoDeParametros;

	*(tamanioTotal) = desplazamiento;

	return unaOperacion;
}

//void *memcpy(void *dest, const void *src, size_t n);
operacionLQL* deserializarOperacionLQL(void* bufferOperacion){
	int yeahIWontUseThis; // i hate C
	operacionLQL *unaOperacion = _deserializarOperacionSinFree(bufferOperacion, &yeahIWontUseThis);
	free(bufferOperacion);
	return unaOperacion;
}

/*
 * Serializa una operacion LQL.
 */
void* serializarOperacionLQL(operacionLQL* operacionLQL, int* tamanio) {

	int desplazamiento = 0;
	int tamanioOperacion = strlen(operacionLQL->operacion) + 1;
	operacionProtocolo protocolo = OPERACIONLQL;
	int tamanioParametros = strlen(operacionLQL->parametros) + 1;
	int tamanioTotalBuffer = 3*sizeof(int) + tamanioOperacion + tamanioParametros;

	void *bufferOperacion= malloc(tamanioTotalBuffer);

	// Operacion de Protocolo
	memcpy(bufferOperacion + desplazamiento, &protocolo, sizeof(int));
	desplazamiento+= sizeof(int);
	// Tamaño de operacion LQL
	memcpy(bufferOperacion + desplazamiento, &tamanioOperacion, sizeof(int));
	desplazamiento+= sizeof(int);
	// operacion LQL
	memcpy(bufferOperacion + desplazamiento, (operacionLQL->operacion), tamanioOperacion);
	desplazamiento+= sizeof(char)*tamanioOperacion;
	// Tamaño de parametros
	memcpy(bufferOperacion + desplazamiento, &tamanioParametros, sizeof(int));
	desplazamiento+= sizeof(int);
	// parametros
	memcpy(bufferOperacion + desplazamiento, (operacionLQL->parametros), tamanioParametros);
	desplazamiento+= tamanioParametros;

	*tamanio = desplazamiento;

	return bufferOperacion;
}

operacionLQL* splitear_operacion(char* operacion){
	operacionLQL* operacionAux = malloc(sizeof(operacionLQL));
	char** opSpliteada;

	if(string_equals_ignore_case(operacion, "JOURNAL") || string_equals_ignore_case(operacion, "DESCRIBE") || string_equals_ignore_case(operacion, "HEXDUMP")) {
		operacionAux->operacion = operacion;
		operacionAux->parametros = malloc(3);
		strcpy(operacionAux->parametros, "ALL");
	} else if (){
		opSpliteada = string_n_split(operacion,2," ");
		operacionAux->operacion=string_duplicate(*opSpliteada);
		operacionAux->parametros=string_duplicate(*(opSpliteada+1));
		free(*opSpliteada);
		free(*(opSpliteada + 1));
		free(opSpliteada);
	}


	return operacionAux;
}

void serializarYEnviarOperacionLQL(int socket, operacionLQL* operacionLQL) {
	int tamanioBuffer;
	void* bufferAEnviar = serializarOperacionLQL(operacionLQL, &tamanioBuffer);
	enviar(socket, bufferAEnviar, tamanioBuffer);
	free(bufferAEnviar);
}

/*
	Serializa una metadata. Toma un parametro:
	unaMetadata: La metadata a serializar
*/
void* serializarMetadata(metadata* unMetadata, int *tamanioBuffer) {

	int desplazamiento = 0;
	int tamanioDelTipoDeConsistencia = sizeof(int);
	int tamanioDeCantidadDeParticiones = sizeof(int);
	int tamanioDelTiempoDeCompactacion = sizeof(int);
	int tamanioDelNombreTabla = strlen(unMetadata->nombreTabla) + 1;

	int tamanioProtocolo = sizeof(int);
	operacionProtocolo protocolo = METADATA;
	int tamanioTotalDelBuffer = 6*sizeof(int) + tamanioDelTipoDeConsistencia + tamanioDeCantidadDeParticiones + tamanioDelTiempoDeCompactacion + tamanioDelNombreTabla;
	void *bufferMetadata= malloc(tamanioTotalDelBuffer);

	//Tamaño de operacion Protocolo
	memcpy(bufferMetadata + desplazamiento, &tamanioProtocolo, sizeof(int));
	desplazamiento += sizeof(int);
	//Operacion de Protocolo
	memcpy(bufferMetadata + desplazamiento, &protocolo, sizeof(int));
	desplazamiento+= sizeof(int);
	//Tamaño del tipo de consistencia
	memcpy(bufferMetadata + desplazamiento, &tamanioDelTipoDeConsistencia, tamanioDelTipoDeConsistencia);
	desplazamiento+= sizeof(int);
	//Tipo de consistencia
	memcpy(bufferMetadata + desplazamiento, &(unMetadata->tipoConsistencia), tamanioDelTipoDeConsistencia);
	desplazamiento+= tamanioDelTipoDeConsistencia;
	//Tamaño de la cantidad de particiones
	memcpy(bufferMetadata + desplazamiento, &tamanioDeCantidadDeParticiones, tamanioDeCantidadDeParticiones);
	desplazamiento+= sizeof(int);
	//Cantidad de particiones
	memcpy(bufferMetadata + desplazamiento, &(unMetadata->cantParticiones), tamanioDeCantidadDeParticiones);
	desplazamiento+= tamanioDeCantidadDeParticiones;
	//Tamaño del tiempoDeCompactacion
	memcpy(bufferMetadata + desplazamiento, &tamanioDelTiempoDeCompactacion, tamanioDelTiempoDeCompactacion);
	desplazamiento+= sizeof(int);
	//Tiempo de compactacion
	memcpy(bufferMetadata + desplazamiento, &(unMetadata->tiempoCompactacion), tamanioDelTiempoDeCompactacion);
	desplazamiento+= tamanioDelTiempoDeCompactacion;
	//tamanio nombre tabla
	memcpy(bufferMetadata + desplazamiento, &tamanioDelNombreTabla, tamanioDelTiempoDeCompactacion);
	desplazamiento += sizeof(int);
	//nombre tabla
	memcpy(bufferMetadata + desplazamiento, (unMetadata->nombreTabla), tamanioDelNombreTabla);
	desplazamiento += tamanioDelNombreTabla;


	*tamanioBuffer = desplazamiento;

	return bufferMetadata;
}

void serializarYEnviarMetadata(int socket, metadata* unaMetadata) {
	int tamanioAEnviar;
	void* bufferAEnviar = serializarMetadata(unaMetadata, &tamanioAEnviar);
	enviar(socket, bufferAEnviar, tamanioAEnviar);
	free(bufferAEnviar);
}

metadata* _deserializarMetadataSinFree(void* bufferMetadata, int *tamanio) {
	int desplazamiento = 8;
	metadata* unMetadata = malloc(sizeof(metadata));
	int tamanioDelTipoDeConsistencia,tamanioDeCantidadDeParticiones,tamanioDelTiempoDeCompactacion, tamanioNombreTabla;

	memcpy(&tamanioDelTipoDeConsistencia, bufferMetadata + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(&(unMetadata->tipoConsistencia), bufferMetadata + desplazamiento, sizeof(int));
	desplazamiento+= sizeof(int);

	memcpy(&tamanioDeCantidadDeParticiones, bufferMetadata + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(&(unMetadata->cantParticiones), bufferMetadata + desplazamiento, sizeof(int));
	desplazamiento+= sizeof(int);

	memcpy(&tamanioDelTiempoDeCompactacion, bufferMetadata + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(&(unMetadata->tiempoCompactacion), bufferMetadata + desplazamiento, sizeof(int));
	desplazamiento+= sizeof(int);

	memcpy(&tamanioNombreTabla, bufferMetadata + desplazamiento, sizeof(int));
	desplazamiento+= sizeof(int);

	unMetadata->nombreTabla = malloc(tamanioNombreTabla);
	memcpy((unMetadata->nombreTabla), bufferMetadata + desplazamiento, tamanioNombreTabla);
	desplazamiento += tamanioNombreTabla;

	*(tamanio) = desplazamiento;

	return unMetadata;
}

metadata* deserializarMetadata(void* bufferMetadata) {
	int yeahIWontUseThis; // I still hate C
	metadata* unMetadata = _deserializarMetadataSinFree(bufferMetadata, &yeahIWontUseThis);
	free(bufferMetadata);
	return unMetadata;
}

char* string_trim_quotation(char* string) {
	char* stringRespuesta = malloc(strlen(string) - 1);
	char* aux = string + 1;
	int i = 1;
	while(*(string + i) != '"') {
		i++;
	}
	i--;
	size_t tamanioString = i;
	strncpy(stringRespuesta, aux, tamanioString);
	*(stringRespuesta + i) = '\0';
	free(string);
	return stringRespuesta;
};

registroConNombreTabla* armarRegistroConNombreTabla(registro* unRegistro, char* nombreTabla) {
	registroConNombreTabla* registroParaEnviar = malloc(sizeof(registroConNombreTabla));

	registroParaEnviar->nombreTabla = string_duplicate(nombreTabla);
	registroParaEnviar->value = string_duplicate(unRegistro->value);
	registroParaEnviar->key = unRegistro->key;
	registroParaEnviar->timestamp = unRegistro->timestamp;

	return registroParaEnviar;
}

void liberarRegistroConNombreTabla(registroConNombreTabla* registro) {
	free(registro->nombreTabla);
	free(registro->value);
	free(registro);
}

void* serializarPaqueteDeOperacionesLQL(t_list* operacionesLQL, int* tamanio) {
	int tamanioTotal = sizeof(operacionProtocolo);
	t_list* buffersAFoldear = list_create();
	void* bufferTotal;
	int desplazamiento = 0;
	operacionProtocolo protocolo = PAQUETEOPERACIONES;

	void* agregarOperacionAListaDeBuffers(operacionLQL *unaOperacion) {
		bufferConTamanio* bufferOperacion = malloc(sizeof(bufferConTamanio));
		bufferOperacion->buffer = serializarOperacionLQL(unaOperacion, &bufferOperacion->tamanio);
		tamanioTotal += bufferOperacion->tamanio;
		list_add(buffersAFoldear, bufferOperacion);
	}

	void* agregarBufferDeOperacionABufferTotal(bufferConTamanio* bufferDeOperacion) {
		memcpy(bufferTotal + desplazamiento, bufferDeOperacion->buffer, bufferDeOperacion->tamanio);
		desplazamiento += bufferDeOperacion->tamanio;
		free(bufferDeOperacion->buffer);
		free(bufferDeOperacion);
	}

	list_iterate(operacionesLQL, agregarOperacionAListaDeBuffers);
	bufferTotal = malloc(tamanioTotal);

	memcpy(bufferTotal, &protocolo, sizeof(operacionProtocolo));
	desplazamiento += sizeof(operacionProtocolo);

	list_iterate(buffersAFoldear, agregarBufferDeOperacionABufferTotal);

	*(tamanio) = tamanioTotal;

	list_destroy(buffersAFoldear);

	return bufferTotal;
}

void serializarYEnviarPaqueteOperacionesLQL(int socket, t_list* operacionesLQL) {
	int tamanioAEnviar;
	void* bufferAEnviar = serializarPaqueteDeOperacionesLQL(operacionesLQL, &tamanioAEnviar);
	enviar(socket, bufferAEnviar, tamanioAEnviar);
	free(bufferAEnviar);
}

void recibirYDeserializarPaqueteDeOperacionesLQLRealizando(int socket, void(*accion)(operacionLQL*)) {
	int desplazamiento = 4;
	int tamanioTotal;
	int tamanioUnaOperacion;
	void* bufferTotal;

	recv(socket, &tamanioTotal, sizeof(int), MSG_WAITALL);
	bufferTotal = malloc(tamanioTotal);
	recv(socket, bufferTotal, tamanioTotal, MSG_WAITALL);

	while(desplazamiento < tamanioTotal) {
		operacionLQL* unaOperacion = _deserializarOperacionSinFree(bufferTotal + desplazamiento, &tamanioUnaOperacion);
		accion(unaOperacion);
		liberarOperacionLQL(unaOperacion);
		desplazamiento += tamanioUnaOperacion;
	}
	free(bufferTotal);
}

void* serializarPaqueteDeMetadatas(t_list* metadatas, int* tamanio) {
	int tamanioTotal = sizeof(operacionProtocolo);
	t_list* buffersAFoldear = list_create();
	void* bufferTotal;
	int desplazamiento = 0;
	operacionProtocolo protocolo = PAQUETEMETADATAS;

	void* agregarOperacionAListaDeBuffers(metadata* unaMetadata) {
		bufferConTamanio* bufferMetadata = malloc(sizeof(bufferConTamanio));
		bufferMetadata->buffer = serializarMetadata(unaMetadata, &bufferMetadata->tamanio);
		tamanioTotal += bufferMetadata->tamanio;
		list_add(buffersAFoldear, bufferMetadata);
	}

	void* agregarBufferDeOperacionABufferTotal(bufferConTamanio* bufferDeOperacion) {
		memcpy(bufferTotal + desplazamiento, bufferDeOperacion->buffer, bufferDeOperacion->tamanio);
		desplazamiento += bufferDeOperacion->tamanio;
		free(bufferDeOperacion->buffer);
		free(bufferDeOperacion);
	}

	list_iterate(metadatas, agregarOperacionAListaDeBuffers);
	bufferTotal = malloc(tamanioTotal);

	memcpy(bufferTotal, &protocolo, sizeof(operacionProtocolo));
	desplazamiento += sizeof(operacionProtocolo);

	list_iterate(buffersAFoldear, agregarBufferDeOperacionABufferTotal);

	*(tamanio) = tamanioTotal;

	list_destroy(buffersAFoldear);

	return bufferTotal;
}

void serializarYEnviarPaqueteMetadatas(int socket, t_list* metadatas) {
	int tamanioAEnviar;
	void* bufferAEnviar = serializarPaqueteDeMetadatas(metadatas, &tamanioAEnviar);
	enviar(socket, bufferAEnviar, tamanioAEnviar);
	free(bufferAEnviar);
}

void recibirYDeserializarPaqueteDeMetadatasRealizando(int socket, void(*accion)(metadata*)) {
	int desplazamiento = 4;
	int tamanioTotal;
	int tamanioUnaMetadata;
	void* bufferTotal;

	recv(socket, &tamanioTotal, sizeof(int), MSG_WAITALL);
	bufferTotal = malloc(tamanioTotal);
	recv(socket, bufferTotal, tamanioTotal, MSG_WAITALL);

	while(desplazamiento < tamanioTotal) {
		metadata* unaMetadata = _deserializarMetadataSinFree(bufferTotal + desplazamiento, &tamanioUnaMetadata);
		accion(unaMetadata);
		free(unaMetadata->nombreTabla);
		free(unaMetadata);
		desplazamiento += tamanioUnaMetadata;
	}
	free(bufferTotal);
}
