/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "node.h"
#include "client.h"
#include "mtnl.h"

#include <stdlib.h>

MTNode* mtnode_create(Client* client) {
    MTNode* node;
    if ((node = calloc(1, sizeof(MTNode))) == NULL)
        return NULL;

    mtnode_init(node, client);
    return node;
}

void mtnode_init(MTNode* node, Client* client) {
    node->client = client;
    node->parent = NULL;
    node->next = NULL;
    node->prev = NULL;
    node->children = calloc(1, sizeof(Children));
}

Client* get_node_client(MTNode* node) {
    if (node == NULL) return NULL;
    return node->client;
}
