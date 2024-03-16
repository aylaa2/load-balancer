/* Copyright 2023 <> */
#include <stdlib.h>
#include <string.h>

#include "server.h"

struct server_memory {
	/* TODO 0 */
}

server_memory *init_server_memory()
{
	/* TODO 1 */
	return NULL;
}

void server_store(server_memory *server, char *key, char *value) {
	/* TODO 2 */
}

char *server_retrieve(server_memory *server, char *key) {
	/* TODO 3 */
	return NULL;
}

void server_remove(server_memory *server, char *key) {
	/* TODO 4 */
}

void free_server_memory(server_memory *server) {
	/* TODO 5 */
}
