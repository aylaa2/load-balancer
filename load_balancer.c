/* Copyright 2023 <> */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "load_balancer.h"

#define REPLICA_CONST   100000
#define MAX_REPLICA 3
#define SERVER_MAX  99999

struct load_balancer {
    /* TODO 0 */
    unsigned int *hash_ring;

    unsigned int hash_ring_size;

    server_memory **servers;
};

unsigned int hash_function_servers(void *a) {
    unsigned int uint_a = *((unsigned int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

unsigned int hash_function_key(void *a) {
    unsigned char *puchar_a = (unsigned char *)a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}

unsigned int get_server_id(unsigned int replica)
{
    unsigned int order = (replica / REPLICA_CONST) * REPLICA_CONST;

    return replica - order;
}

int cmp_hash_ring(void *a, void *b)
{
    unsigned int hash_a, hash_b;
    int server_id_a, server_id_b;

    hash_a = hash_function_servers(a);
    hash_b = hash_function_servers(b);

    if (hash_a == hash_b) {
        server_id_a = get_server_id(*((unsigned int *)a));
        server_id_b = get_server_id(*((unsigned int *)b));

        return server_id_a - server_id_b;
    }

    return hash_a > hash_b ? 1 : -1;
}

unsigned int get_placement_idx(load_balancer *main, char *key)
{
    int left = 0;
    int right = main->hash_ring_size - 1;
    int middle;

    unsigned int key_hash = hash_function_key(key);
    unsigned int server_hash;

    while (left <= right) {
        middle = left + (right - left)/2;

        server_hash = hash_function_servers(&main->hash_ring[middle]);

        if (key_hash == server_hash)
            return main->hash_ring[middle];
        else if (key_hash > server_hash)
            left = middle + 1;
        else
            right = middle - 1;
    }
    return main->hash_ring[left % main->hash_ring_size];
}

unsigned int get_neighbour(load_balancer *main, unsigned int replica)
{
    int left = 0;
    int right = main->hash_ring_size - 1;
    int middle;

    unsigned int replica_hash = hash_function_servers(&replica);
    unsigned int server_hash;

    while (left <= right) {
        middle = left + (right - left) / 2;

        server_hash = hash_function_servers(&main->hash_ring[middle]);

        if (replica_hash == server_hash)
            return main->hash_ring[(middle + 1) % main->hash_ring_size];
        else if (replica_hash > server_hash)
            left = middle + 1;
        else
            right = middle - 1;
    }

    return -1;
}

// O(n) vector remove operation
void hash_ring_pop(load_balancer *main, unsigned int value)
{
    unsigned int idx;

    for (idx = 0; idx < main->hash_ring_size; idx++)
        if (main->hash_ring[idx] == value)
            break;

    for (; idx < main->hash_ring_size - 1; idx++)
        main->hash_ring[idx] = main->hash_ring[idx + 1];

    main->hash_ring_size--;
}

void print_hash_ring(load_balancer *main)
{
    for (unsigned int i = 0; i < main->hash_ring_size; i++)
        printf("%u ", main->hash_ring[i]);
    printf("\n");
}

void distribute_pairs(load_balancer *main,
                        info **pairs, unsigned int pairs_size)
{
    unsigned int idx;
    int new_server_id;
    char *key, *value;

    for (idx = 0; idx < pairs_size; idx++) {
        key = get_info_key(pairs[idx]);
        value = get_info_value(pairs[idx]);

        loader_store(main, key, value, &new_server_id);
        free(key);
        free(value);
        free(pairs[idx]);
    }

    free(pairs);
}

load_balancer *init_load_balancer() {
    /* TODO 1 */
    load_balancer *main = malloc(sizeof(load_balancer));

    main->hash_ring = calloc(SERVER_MAX, sizeof(unsigned int));

    main->hash_ring_size = 0;

    main->servers = malloc(SERVER_MAX * sizeof(server_memory *));

    for (int i = 0; i < SERVER_MAX; i++)
        main->servers[i] = NULL;

    return main;
}

void loader_add_server(load_balancer *main, int server_id) {
    /* TODO 2 */
    unsigned int *replicas = malloc(MAX_REPLICA * sizeof(unsigned int));
    unsigned int idx, server_size;
    unsigned int neighbour_id;
    info **pairs;

    /* Populating the replica server */
    for (idx = 0; idx < MAX_REPLICA; idx++)
        replicas[idx] = idx * REPLICA_CONST + server_id;

    /* Generate replicas and add them to the hash ring */
    for (unsigned int idx = 0; idx < MAX_REPLICA; idx++) {
        replicas[idx] = idx * REPLICA_CONST + server_id;
        main->hash_ring[main->hash_ring_size++] = replicas[idx];
    }

    /* Resorting the hash ring */
    qsort(main->hash_ring, main->hash_ring_size,
            sizeof(unsigned int), cmp_hash_ring);

    main->servers[server_id] = init_server_memory();

    for (idx = 0; idx < MAX_REPLICA; idx++) {
        neighbour_id = get_server_id(get_neighbour(main, replicas[idx]));

        server_size = get_server_size(main->servers[neighbour_id]);

        pairs = server_retrieve_all(main->servers[neighbour_id]);

        free_server_memory(main->servers[neighbour_id]);

        main->servers[neighbour_id] = init_server_memory();

        distribute_pairs(main, pairs, server_size);
    }

    free(replicas);
}

void loader_remove_server(load_balancer *main, int server_id) {
    /* TODO 3 */
    unsigned int idx;
    unsigned int deleted_server_size;
    info **pairs;

    /* Eliminating the server id and all the replicas from the hash ring */
    for (idx = 0; idx < MAX_REPLICA; idx++)
        hash_ring_pop(main, idx * REPLICA_CONST + server_id);

    deleted_server_size = get_server_size(main->servers[server_id]);

    pairs = server_retrieve_all(main->servers[server_id]);

    distribute_pairs(main, pairs, deleted_server_size);

    free_server_memory(main->servers[server_id]);

    main->servers[server_id] = NULL;
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id) {
    /* TODO 4 */
    unsigned int idx = get_server_id(get_placement_idx(main, key));

    *server_id = idx;

    server_store(main->servers[idx], key, value);
}

char *loader_retrieve(load_balancer *main, char *key, int *server_id) {
    /* TODO 5 */
    unsigned int idx = get_server_id(get_placement_idx(main, key));

    *server_id = idx;

    return server_retrieve(main->servers[idx], key);
}

void free_load_balancer(load_balancer *main) {
    /* TODO 6 */
    int idx;

    free(main->hash_ring);

    for (idx = 0; idx < SERVER_MAX; idx++)
        if (main->servers[idx] != NULL)
            free_server_memory(main->servers[idx]);

    free(main->servers);
    free(main);
}
