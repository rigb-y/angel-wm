/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_MINIMIZED_CLIENT_H
#define ANGEL_MINIMIZED_CLIENT_H

#include "types.h"

typedef struct Client Client;

typedef struct MinimizedClient {
    Client* client;
    struct MinimizedClient* next;
    struct MinimizedClient* prev;
    int position_in_client_list;
    ClientListOrigin origin;
} MinimizedClient;

MinimizedClient* create_minimized_client(Client*, int, ClientListOrigin);
MinimizedClient* minimize_client(Client*);
Client* get_client_from_minimized(MinimizedClient*);
int get_client_pos_from_minimized(MinimizedClient*);
void set_minimized_list_origin(MinimizedClient*, ClientListOrigin);
ClientListOrigin get_minimized_list_origin(MinimizedClient*);

#endif
