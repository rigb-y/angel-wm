/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_DETACHED_H
#define ANGEL_DETACHED_H

#include "types.h"

typedef struct Client Client;

// The client list owns whichever DetachedClient is currently set
// If we replace the cls detached client, we must free it
// (but not the client it holds).
typedef struct DetachedClient {
  Client* client;
  struct DetachedClient* next;
  int position_in_client_list;

  int x, y;
  int width, height;

  _Bool in_move_mode;
  _Bool in_mouse_resize_mode;
  _Bool configured;
  int pointer_x;
  int pointer_y;
  ClientListOrigin origin;
} DetachedClient;

DetachedClient* create_detached_client(Client*, int, ClientListOrigin);
DetachedClient* detach_client(Client*);
Client* get_client_from_detached(DetachedClient*);
int get_client_pos_from_detached(DetachedClient*);
_Bool get_move_mode(DetachedClient*);
void set_move_mode(DetachedClient*, _Bool);
void set_mouse_resize_mode(DetachedClient*, _Bool);
_Bool get_mouse_resize_mode(DetachedClient*);

void set_detached_configured(DetachedClient*, _Bool);
_Bool get_detached_configured(DetachedClient*);

void set_detached_pointer_xy(DetachedClient*, int, int);
int get_detached_pointer_x(DetachedClient*);
int get_detached_pointer_y(DetachedClient*);

void set_detached_list_origin(DetachedClient*, ClientListOrigin);
ClientListOrigin get_detached_list_origin(DetachedClient*);

int get_detached_x(DetachedClient*);
int get_detached_y(DetachedClient*);
int get_detached_width(DetachedClient*);
int get_detached_height(DetachedClient*);

void set_detached_x(DetachedClient*, int);
void set_detached_y(DetachedClient*, int);
void set_detached_width(DetachedClient*, int);
void set_detached_height(DetachedClient*, int);

#endif
