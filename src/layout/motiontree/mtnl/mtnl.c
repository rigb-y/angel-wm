/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "mtnl.h"

#include <stdbool.h>
#include <stdlib.h>

static inline _Bool invalid_child_number(Children* list, int n) {
    if (list == NULL) return NULL;

    return n <= 0 || n > list->n;
}

size_t mtnl_size(Children* list) {
    if (list == NULL) return 0;
    return list->n;
}

_Bool mtnl_empty(Children* list) {
    if (list == NULL) return true;
    return mtnl_size(list) == 0;
}

_Bool mtnl_single(Children* list) {
    if (list == NULL) return false;
    return mtnl_size(list) == 1;
}

MTNode* mtnl_find(Children* list, MTNode* node) {
    if (list == NULL || node == NULL) return NULL;

    MTNode* curr = list->head;
    while (curr != NULL) {
        if (curr == node) return curr;
        curr=curr->next;
    }

    return NULL;
}

MTNode* mtnl_get_child_n(Children* list, int n) {
    if (list == NULL || invalid_child_number(list, n)) return NULL;

    MTNode* curr = list->head;
    int pos = 1;
    while (curr != NULL && pos < n) {
        curr = curr->next;
        ++pos;
    }

    return curr;
}

int mtnl_child_number(Children* list, MTNode* node) {
    if (list == NULL || node == NULL || mtnl_find(list, node) == NULL) return -1;

    MTNode* curr = list->head;
    int pos = 1;
    while (curr != NULL && curr != node) {
        curr = curr->next;
        ++pos;
    }
    return pos;
}

void mtnl_append(Children* list, MTNode* node) {
    if (list == NULL || node == NULL || mtnl_find(list, node) != NULL) return;

    MTNode* curr = list->head;
    while (curr != NULL && curr->next != NULL) {
        curr=curr->next;
    }
    mtnl_insert_after(list, curr, node);
}

void mtnl_prepend(Children* list, MTNode* node) {
    if (list == NULL || node == NULL || mtnl_find(list, node) != NULL) return;

    node->prev = NULL;
    node->next = NULL;

    if (mtnl_empty(list)) {
        list->head = node;

        ++list->n;
        return;
    }

    node->next = list->head;
    list->head->prev = node;
    list->head = node;

    ++list->n;
}

void mtnl_insert_after(Children* list, MTNode* sibling, MTNode* node) {
    if (list == NULL || node == NULL) return;

    // If the node is already in the list 
    if (mtnl_find(list, node) != NULL) return;

    node->next = NULL;
    node->prev = NULL;

    if (sibling == NULL) { 
        if (mtnl_empty(list)) {
            list->head = node;
            ++list->n;
        }
        return;
    }

    // Sibling not in the list
    if (mtnl_find(list, sibling) == NULL) return;

    MTNode* sibling_next = sibling->next;
    sibling->next = node;
    node->prev = sibling;

    if (sibling_next) {
        node->next = sibling_next;
        sibling_next->prev = node;
    }

    ++list->n;
}

void mtnl_remove(Children* list, MTNode** node) {
    if (list == NULL || 
        node == NULL || 
        *node == NULL ||
        mtnl_find(list, *node) == NULL
    ) return;

    if (*node == list->head) {
        list->head = list->head->next;
        list->head->prev = NULL;

        free(*node);
        *node = NULL;
        --list->n;

        return;
    }

    // node_parent is non-NULL since node is not head
    MTNode* node_parent = (*node)->prev;
    MTNode* node_next = (*node)->next;

    node_parent->next = node_next;

    if (node_next) node_next->prev = node_parent;

    free(*node);
    *node = NULL;
    --list->n;
}

void mtnl_destroy(Children* list) {
    if (list == NULL) return;

    MTNode* curr = list->head;
    while (curr != NULL) {
        MTNode* tmp = curr;
        curr=curr->next;

        tmp->prev = NULL;
        tmp->next = NULL;
        free(tmp);
    }

    list->head = NULL;
    list->n = 0;
}
