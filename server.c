/* Copyright 2023 <> */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "server.h"
#include "utils.h"

#define HMAX 10

/* --------------- Linked List Implementation ------------------ */

typedef struct ll_node_t
{
    void* data;
    struct ll_node_t* next;
} ll_node_t;

typedef struct linked_list_t
{
    ll_node_t* head;
    unsigned int data_size;
    unsigned int size;
} linked_list_t;

linked_list_t *
ll_create(unsigned int data_size)
{
    linked_list_t* ll;

    ll = malloc(sizeof(*ll));
    DIE(ll == NULL, "linked_list malloc");

    ll->head = NULL;
    ll->data_size = data_size;
    ll->size = 0;

    return ll;
}


void
ll_add_nth_node(linked_list_t* list, unsigned int n, const void* new_data)
{
    ll_node_t *prev, *curr;
    ll_node_t* new_node;

    if (!list) {
        return;
    }

    /* n >= list->size inseamna adaugarea unui nou nod la finalul listei. */
    if (n > list->size) {
        n = list->size;
    }

    curr = list->head;
    prev = NULL;
    while (n > 0) {
        prev = curr;
        curr = curr->next;
        --n;
    }

    new_node = malloc(sizeof(*new_node));
    DIE(new_node == NULL, "new_node malloc");
    new_node->data = malloc(list->data_size);
    DIE(new_node->data == NULL, "new_node->data malloc");
    memcpy(new_node->data, new_data, list->data_size);

    new_node->next = curr;
    if (prev == NULL) {
        /* Adica n == 0. */
        list->head = new_node;
    } else {
        prev->next = new_node;
    }

    list->size++;
}


ll_node_t *
ll_remove_nth_node(linked_list_t* list, unsigned int n)
{
    ll_node_t *prev, *curr;

    if (!list || !list->head) {
        return NULL;
    }

    /* n >= list->size - 1 inseamna eliminarea nodului de la finalul listei. */
    if (n > list->size - 1) {
        n = list->size - 1;
    }

    curr = list->head;
    prev = NULL;
    while (n > 0) {
        prev = curr;
        curr = curr->next;
        --n;
    }

    if (prev == NULL) {
        /* Adica n == 0. */
        list->head = curr->next;
    } else {
        prev->next = curr->next;
    }

    list->size--;

    return curr;
}


unsigned int
ll_get_size(linked_list_t* list)
{
     if (!list) {
        return -1;
    }

    return list->size;
}


void
ll_free(linked_list_t** pp_list)
{
    ll_node_t* currNode;

    if (!pp_list || !*pp_list) {
        return;
    }

    while (ll_get_size(*pp_list) > 0) {
        currNode = ll_remove_nth_node(*pp_list, 0);
        free(currNode->data);
        currNode->data = NULL;
        free(currNode);
        currNode = NULL;
    }

    free(*pp_list);
    *pp_list = NULL;
}


void
ll_print_int(linked_list_t* list)
{
    ll_node_t* curr;

    if (!list) {
        return;
    }

    curr = list->head;
    while (curr != NULL) {
        printf("%d ", *((int*)curr->data));
        curr = curr->next;
    }

    printf("\n");
}


void
ll_print_string(linked_list_t* list)
{
    ll_node_t* curr;

    if (!list) {
        return;
    }

    curr = list->head;
    while (curr != NULL) {
        printf("%s ", (char*)curr->data);
        curr = curr->next;
    }

    printf("\n");
}

/* ------------ END Linked List Implementation ----------------- */

/* ------------ Server Implementation -------------------------- */

unsigned int
hash_function_string(void *a)
{
	/*
	 * Credits: http://www.cse.yorku.ca/~oz/hash.html
	 */
	unsigned char *puchar_a = (unsigned char*) a;
	unsigned long hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c; /* hash * 33 + c */

	return hash;
}

int
compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

struct info {
	void *key;
	void *value;
};

char *get_info_key(info *pair)
{
	return (char *)pair->key;
}

char *get_info_value(info *pair)
{
	return (char *)pair->value;
}

void free_info(info *pair)
{
	free(pair->key);
	free(pair->value);
	free(pair);
}

struct server_memory {
	/* TODO 0 */
	linked_list_t **buckets;

	unsigned int size;
	unsigned int hmax;

	unsigned int (*hash_function)(void *);
	int (*compare_function)(void *, void *);
};

server_memory *init_server_memory()
{
	/* TODO 1 */
	server_memory *server = malloc(sizeof(server_memory));

	server->size = 0;
	server->hmax = HMAX;

	server->buckets = (linked_list_t **)malloc(HMAX * sizeof(linked_list_t *));
	for (unsigned int i = 0; i < HMAX; i++)
		server->buckets[i] = ll_create(sizeof(info));

	server->hash_function = hash_function_string;
	server->compare_function = compare_function_strings;

	return server;
}

void server_store(server_memory *server, char *key, char *value) {
	/* TODO 2 */
	unsigned int idx;
	info *pair;
	size_t key_size, value_size;

	key_size = strlen(key) + 1;
	value_size = strlen(value) + 1;

	pair = malloc(sizeof(info));
	pair->key = malloc(key_size);
	pair->value = malloc(value_size);
	memcpy(pair->key, key, key_size);
	memcpy(pair->value, value, value_size);

	idx = server->hash_function(key) % server->hmax;

	ll_add_nth_node(server->buckets[idx], server->buckets[idx]->size, pair);

	server->size++;

	free(pair);
}

int server_has_key(server_memory *server, char *key)
{
	unsigned int idx;
	ll_node_t *p;

	idx = server->hash_function(key) % server->hmax;

	for (p = server->buckets[idx]->head; p != NULL; p = p->next)
		if (server->compare_function(((info *)p->data)->key, key) == 0)
			return 1;

	return 0;
}

char *server_retrieve(server_memory *server, char *key) {
	/* TODO 3 */
	unsigned int idx;
	ll_node_t *p;

	idx = server->hash_function(key) % server->hmax;

	for (p = server->buckets[idx]->head; p != NULL; p = p->next)
		if (server->compare_function(((info *)p->data)->key, key) == 0)
			return (char*)((info *)p->data)->value;

	return NULL;
}

info **server_retrieve_all(server_memory *server)
{
	info **pairs = malloc(server->size * sizeof(info *));
	unsigned int idx, n = 0;
	char *key, *value;
	size_t key_size, value_size;
	ll_node_t *p;

	for (idx = 0; idx < server->hmax; idx++) {
		for (p = server->buckets[idx]->head; p != NULL; p = p->next) {
			key = ((info *)p->data)->key;
			value = ((info *)p->data)->value;
			key_size = strlen(key) + 1;
			value_size = strlen(value) + 1;

			pairs[n] = malloc(sizeof(info));
			pairs[n]->key = malloc(key_size);
			pairs[n]->value = malloc(value_size);
			memcpy(pairs[n]->key, ((info *)p->data)->key, key_size);
			memcpy(pairs[n]->value, ((info *)p->data)->value, value_size);
			n++;
		}
	}

	return pairs;
}

void server_remove(server_memory *server, char *key) {
	/* TODO 4 */
	unsigned int idx,  list_idx;
	ll_node_t *deleted_node, *p;

	if (!server_has_key(server, key))
		return;

	idx = server->hash_function(key) % server->hmax;

	for (p = server->buckets[idx]->head, list_idx = 0;
			p != NULL; p = p->next, list_idx++)
		if (server->compare_function(((info *)p->data)->key, key) == 0)
			break;

	deleted_node = ll_remove_nth_node(server->buckets[idx], list_idx);

	free(((info *)deleted_node->data)->key);

	free(((info *)deleted_node->data)->value);

	free(deleted_node);

	server->size--;
}

void free_server_memory(server_memory *server) {
	/* TODO 5 */
	unsigned int idx;
	ll_node_t *p;

	for (idx = 0; idx < server->hmax; idx++) {
		for (p = server->buckets[idx]->head; p != NULL; p = p->next) {
			free(((info *)p->data)->key);
			free(((info *)p->data)->value);
		}
		ll_free(&server->buckets[idx]);
	}

	free(server->buckets);

	free(server);
}

unsigned int get_server_size(server_memory *server)
{
	return server->size;
}
