/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_UNMAPPED_CLIENT_H
#define ANGEL_UNMAPPED_CLIENT_H

#include "types.h"

#define STAY_UNMAPPED true
#define CAN_BE_MAPPED false

typedef struct Client Client;

typedef struct UnmappedClient {
    Client* client;
    struct UnmappedClient* next;
    int position_in_client_list;
    ClientListOrigin origin;
    ClientListOrigin origin_origin;

    _Bool stay_unmapped;
} UnmappedClient;

UnmappedClient* create_unmapped_client(Client*, int, ClientListOrigin, ClientListOrigin);
UnmappedClient* detach_unmapped(Client*);

Client* get_client_from_unmapped(UnmappedClient*);
int get_client_pos_from_unmapped(UnmappedClient*);

void set_unmapped_list_origin(UnmappedClient*, ClientListOrigin);
ClientListOrigin get_unmapped_list_origin(UnmappedClient*);

void set_unmapped_list_origin_origin(UnmappedClient*, ClientListOrigin);
ClientListOrigin get_unmapped_list_origin_origin(UnmappedClient*);

void unmapped_set_stay_unmapped(UnmappedClient*, _Bool);
_Bool unmapped_get_stay_unmapped(UnmappedClient*);

#endif
