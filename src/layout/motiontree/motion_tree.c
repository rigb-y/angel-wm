/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "motion_tree.h"
#include "mtnl.h"
#include "node.h"
#include "windows.h"
#include "client.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

static _Bool r_mt_find(MTNode*, MTNode*);
static MTNode* r_mt_find_node_carrying_client(MTNode*, Client*);
static void r_mt_dump(MTNode*, int);
static int r_depth_of_node(MTNode*, MTNode*, _Bool*);
static MTNode* r_mt_descend_left(MTNode*, int, int);
static MTNode* r_mt_descend_right(MTNode*, int, int);

void mt_init(MotionTree* tree) {
    if (tree == NULL) return;
    mt_set_root(tree);
}

void mt_set_root(MotionTree* tree) {
    if (tree == NULL) return;

    MTNode* root_node;
    if ((root_node = calloc(1, sizeof(MTNode))) == NULL)
        return;

    mtnode_init(root_node, NULL);
    tree->root = root_node;
}

MTNode* mt_root(MotionTree* tree) {
    if (tree == NULL) return NULL;
    return tree->root;
}

_Bool r_mt_find(MTNode* curr, MTNode* target) {
    if (curr == NULL) return false;
    if (curr == target) return true;

    Children* children = curr->children;
    if (children == NULL) return false;

    MTNode* next_child = children->head;

    while (next_child != NULL) {
        if (r_mt_find(next_child, target)) 
            return true;
        next_child = next_child->next;
    }

    return false;
}

_Bool mt_find(MotionTree* tree, MTNode* node) {
    if (tree == NULL || node == NULL || tree->root == NULL) return false;

    return r_mt_find(tree->root, node);
}

MTNode* r_mt_find_node_carrying_client(MTNode* curr, Client* client) {
    if (curr == NULL) return NULL;
    if (curr->client == client) return curr;

    MTNode* next_child = curr->children->head;
    while (next_child != NULL) {
        MTNode* inspect = r_mt_find_node_carrying_client(next_child, client);
        if (inspect != NULL) {
            return inspect;
        }
        next_child = next_child->next;
    }

    return NULL;
}

MTNode* mt_find_node_carrying_client(MotionTree* tree, Client* client) {
    if (tree == NULL || client == NULL) return NULL;  
    return r_mt_find_node_carrying_client(tree->root, client);
}

void mt_insert_right_of(MotionTree* tree, MTNode* node, MTNode* target) {
    if (tree == NULL || 
        node == NULL || 
        target == NULL || 
        !mt_find(tree, node) || 
        mt_find(tree, target)
    ) return;

    mtnl_insert_after(node->parent->children, node, target);
    target->parent = node->parent;
}

void mt_insert_below_of(MotionTree* tree, MTNode* node, MTNode* target) {
    if (tree == NULL || 
        node == NULL || 
        target == NULL || 
        !mt_find(tree, node) || 
        mt_find(tree, target)
    ) return;

    mtnl_append(node->children, target);
    target->parent = node;
}

MTNode* mt_left_of(MotionTree* tree, MTNode* node) {
    if (tree == NULL || node == NULL || !mt_find(tree, node)) return NULL;

    MTNode* prev = node->prev;
    if (prev != NULL) {
        return prev;
    }

    // Node is root, guards nothing but best to be uniform (see mt_right_of).
    if (node->parent == NULL) {
        return NULL;
    }

    MTNode* curr = node, *parent = curr->parent;
    while (parent != NULL) {
        if (parent->prev != NULL) {
            MTNode* left_child = parent->prev;
            return mt_descend_right_max_depth(tree, left_child, mt_depth_of_node(tree, node));
        }

        curr = parent;
        parent = curr->parent;
    }

    return NULL;
}

MTNode* mt_right_of(MotionTree* tree, MTNode* node) {
    if (tree == NULL || node == NULL || !mt_find(tree, node)) return NULL;

    MTNode* next = node->next;
    if (next != NULL) {
        return next;
    }

    // Node is root, guards node->parent->children.
    if (node->parent == NULL) {
        return NULL;
    }

    MTNode* curr = node, *parent = curr->parent;
    while (parent != NULL) {
        if (parent->next != NULL) {
            MTNode* right_child = parent->next;
            return mt_descend_left_max_depth(tree, right_child, mt_depth_of_node(tree, node));
        }

        curr = parent;
        parent=curr->parent;
    }

    return NULL;
}

MTNode* mt_above_of(MotionTree* tree, MTNode* node) {
    if (tree == NULL || node == NULL || !mt_find(tree, node)) return NULL;
    return node->parent;
}

MTNode* mt_below_of(MotionTree* tree, MTNode* node) {
    if (tree == NULL || node == NULL || !mt_find(tree, node)) return NULL;
    return node->children->head;
}

void r_mt_destroy(MTNode* curr) {
    if (curr == NULL) return;

    MTNode* next_child = curr->children->head;
    while (next_child != NULL) {
        r_mt_destroy(next_child);
        next_child = next_child->next;

    }
    mtnl_destroy(curr->children);
    free(curr->children);
    curr->children=NULL;
}

void mt_destroy(MotionTree* tree) {
    if (tree == NULL) return;
    r_mt_destroy(tree->root);
    free(tree->root);
}

void r_mt_dump(MTNode* curr, int depth) {
    if (curr == NULL) return;

    bool curr_client = curr->client != NULL;
    char* win_name = curr_client 
        ? get_window_name(
            get_client_win(get_node_client(curr))
        ) 
        : "NULL CLIENT";

    for (int i=0; i<depth; ++i) printf("\t");
    printf("%s\n", win_name);

    if (curr_client) free(win_name);

    Children* children = curr->children;
    if (children == NULL) return;

    MTNode* next_child = children->head;
    while (next_child != NULL) {
        r_mt_dump(next_child, depth+1);
        next_child = next_child->next;
    }
}

void mt_dump(MotionTree* tree) {
    if (tree == NULL) return;
    r_mt_dump(tree->root, 0);
}

int r_depth_of_node(MTNode* curr, MTNode* node, _Bool* exit_flag) {
    if (curr == NULL) return 0;

    if (curr == node) {
        *exit_flag = true;
        return 0;
    }

    if (curr->children == NULL) return 0;

    MTNode* next_child = curr->children->head;
    while (next_child != NULL) {
        int path_depth = 1 + r_depth_of_node(next_child, node, exit_flag);
        if (*exit_flag == true) {
            return path_depth;
        }
        next_child = next_child->next;
    }
    return 0;
}

int mt_depth_of_node(MotionTree* tree, MTNode* node) {
    if (tree == NULL || node == NULL || !mt_find(tree, node)) return 0;

    _Bool flag = false;
    return r_depth_of_node(tree->root, node, &flag);
}

// Invariant: Curr will never be null
MTNode* r_mt_descend_left(MTNode* curr, int curr_depth, int max_depth) {
    Children* children = curr->children;
    if (children == NULL || 
        children->head == NULL || 
        curr_depth == max_depth
    ) return curr;

    return r_mt_descend_left(children->head, curr_depth+1, max_depth);
}

MTNode* mt_descend_left_max_depth(MotionTree* tree, MTNode* node, int depth) {
    if (tree == NULL || node == NULL || !mt_find(tree, node)) return NULL;

    int node_depth = mt_depth_of_node(tree, node);
    return r_mt_descend_left(node, node_depth, depth);
}


// Invariant: Curr will never be null
MTNode* r_mt_descend_right(MTNode* curr, int curr_depth, int max_depth) {
    Children* children = curr->children;
    if (children == NULL || 
        children->head == NULL || 
        curr_depth == max_depth
    ) return curr;

    return r_mt_descend_right(mtnl_get_child_n(children, children->n), curr_depth+1, max_depth);
}

MTNode* mt_descend_right_max_depth(MotionTree* tree, MTNode* node, int depth) {
    if (tree == NULL || node == NULL || !mt_find(tree, node)) return NULL;

    int node_depth = mt_depth_of_node(tree, node);
    return r_mt_descend_right(node, node_depth, depth);
}
