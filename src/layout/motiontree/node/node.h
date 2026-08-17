/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_NODE_H
#define ANGEL_NODE_H

typedef struct Client Client;
typedef struct Children Children;

typedef struct MTNode {
    Client* client;
    // For use in motion tree
    struct MTNode* parent;

    // The tree owns all nodes. Each node owns the 
    // list of children but not the children themselves.
    Children* children;
    // For use in linked list
    struct MTNode* next, *prev;
} MTNode;

MTNode* mtnode_create(Client*);
void mtnode_init(MTNode*, Client*);
Client* get_node_client(MTNode*);

#endif
