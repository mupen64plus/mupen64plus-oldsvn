/**
 * Mupen64 - util.h
 * Copyright (C) 2002 Hacktarux
 *
 * Mupen64 homepage: http://mupen64.emulation64.com
 * email address: hacktarux@yahoo.fr
 * 
 * If you want to contribute to the project please contact
 * me first (maybe someone is already making what you are
 * planning to do).
 *
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
 * You should have received a copy of the GNU General Public
 * Licence along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
**/

/**
 * Utility structures/functions. See util.c for function documenation.
 */

#ifndef __UTIL_H__
#define __UTIL_H__

#include <SDL.h>

/** string utilities **/
char *trim(char *str);
char *event_to_str(const SDL_Event *event);
int event_active(const char *event_str);
int key_pressed(SDLKey k);

/** file utilities **/
int isfile(char *path);
int isdir(char *path);
int copyfile(char *src, char *dest);

/** linked list utilities **/
typedef struct _list_node {
    void *data;
    struct _list_node *prev;
    struct _list_node *next;
} list_node_t;

typedef list_node_t * list_t;

list_node_t *list_append(list_t *list, void *data);
void list_node_delete(list_t *list, list_node_t *node);
void list_delete(list_t *list);
void list_node_move_front(list_t *list, list_node_t *node);
void list_node_move_back(list_t *list, list_node_t *node);
list_node_t *list_nth_node(list_t list, int n);
list_node_t *list_last_node(list_t list);
int inline list_empty(list_t list);
int list_length(list_t list);

// cycles through each listnode in list setting curr_node to current node.
#define list_foreach(list, curr_node) \
    for((curr_node) = (list); (curr_node) != NULL; (curr_node) = (curr_node)->next)

#endif // __UTIL_H__
