/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_MOTION_TREE_H
#define ANGEL_MOTION_TREE_H

#include <stddef.h>

typedef struct MTNode MTNode;
typedef struct Client Client;

typedef struct MotionTree {
    MTNode* root;
} MotionTree; 

_Bool mt_find(MotionTree*, MTNode*);
MTNode* mt_find_node_carrying_client(MotionTree*, Client*);

void mt_insert_right_of(MotionTree*, MTNode*, MTNode*);
void mt_insert_below_of(MotionTree*, MTNode*, MTNode*);

void mt_init(MotionTree*);
void mt_set_root(MotionTree*);
MTNode* mt_root(MotionTree*);

MTNode* mt_left_of(MotionTree*, MTNode*);
MTNode* mt_right_of(MotionTree*, MTNode*);
MTNode* mt_above_of(MotionTree*, MTNode*);
MTNode* mt_below_of(MotionTree*, MTNode*);

void mt_destroy(MotionTree*);
void mt_dump(MotionTree*);
int mt_depth_of_node(MotionTree*, MTNode*);
MTNode* mt_descend_left_max_depth(MotionTree*, MTNode*, int);
MTNode* mt_descend_right_max_depth(MotionTree*, MTNode*, int);

#endif
