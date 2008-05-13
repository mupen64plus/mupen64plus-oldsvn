/**
 * Mupen64 - util.c
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
 * Provides common utilities to the rest of the code:
 *  -String functions
 *  -Doubly-linked list
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <SDL.h>

#include "util.h"

/** trim
 *    Removes leading and trailing whitespace from str. Function modifies str
 *    and also returns modified string.
 */
char *trim(char *str)
{
    char *p = str;

    while (isspace(*p))
        p++;

    if(str != p)
        strcpy(str, p);

    p = str + strlen(str) - 1;
    if (p > str)
    {
        while (isspace(*p))
            p--;
        *(++p) = '\0';
    }

    return str;
}

/** event_to_str
 *    Creates a string representation of an SDL input event. If the event is
 *    not supported by this function, NULL is returned.
 *
 *    Notes:
 *     -This function assumes SDL events are already initialized.
 *     -It is up to the caller to free the string memory allocated by this
 *      function.
 */
char *event_to_str(const SDL_Event *event)
{
    char *event_str = NULL;

    switch(event->type)
    {
        case SDL_JOYAXISMOTION:
            if(event->jaxis.value >= 15000 || event->jaxis.value <= -15000)
            {
                event_str = malloc(10);
                snprintf(event_str, 10, "J%dA%d%c",
                         event->jaxis.which,
                     event->jaxis.axis,
                     event->jaxis.value > 0? '+' : '-');
            }
            break;

        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
            event_str = malloc(10);
            snprintf(event_str, 10, "J%dB%d",
                     event->jbutton.which,
                     event->jbutton.button);
            break;

        case SDL_JOYHATMOTION:
            event_str = malloc(10);
            snprintf(event_str, 10, "J%dH%dV%d",
                     event->jhat.which,
                     event->jhat.hat,
                     event->jhat.value);
            break;
    }

    return event_str;
}

/** event_active
 *    Returns 1 if the specified joystick event is currently active. This
 *    function expects an input string of the same form output by event_to_str.
 */
int event_active(const char *event_str)
{
    char device, joy_input_type, axis_direction;
    int dev_number, input_number, input_value;
    SDL_Joystick *joystick = NULL;

    // empty string
    if(!event_str || strlen(event_str) == 0) return 0;

    // joystick event
    if(event_str[0] == 'J')
    {
        // parse string depending on type of joystick input
        switch(event_str[2])
        {
            // axis
            case 'A':
                sscanf(event_str, "%c%d%c%d%c", &device, &dev_number,
                       &joy_input_type, &input_number, &axis_direction);
                break;
            // hat
            case 'H':
                sscanf(event_str, "%c%d%c%dV%d", &device, &dev_number,
                       &joy_input_type, &input_number, &input_value);
                break;
            // button
            case 'B':
                sscanf(event_str, "%c%d%c%d", &device, &dev_number,
                       &joy_input_type, &input_number);
                break;
        }

        joystick = SDL_JoystickOpen(dev_number);
        SDL_JoystickUpdate();
        switch(joy_input_type)
        {
            case 'A':
                if(axis_direction == '-')
                    return SDL_JoystickGetAxis(joystick, input_number) < -15000;
                else
                    return SDL_JoystickGetAxis(joystick, input_number) > 15000;
                return (int)SDL_JoystickGetButton(joystick, input_number);
                break;
            case 'B':
                return (int)SDL_JoystickGetButton(joystick, input_number);
                break;
            case 'H':
                return SDL_JoystickGetHat(joystick, input_number) == input_value;
                break;
            default:
                return 0;
                break;
        }
    }

    // keyboard event
    if(event_str[0] == 'K')
    {
        // TODO
    }
}

/** key_pressed
 *   Returns 1 if the given key is currently pressed.
 */
int key_pressed(SDLKey k)
{
    Uint8 *key_states;
    int num_keys;

    SDL_PumpEvents(); // update input state array
    key_states = SDL_GetKeyState(&num_keys);

    if(k >= num_keys)
        return 0;

    return key_states[k];
}

/** file utilities **/

/** isfile
 *    Returns TRUE if given file path exists and is a regular file
 */
int isfile(char *path)
{
    struct stat sbuf;

    return (stat(path, &sbuf) == 0) && S_ISREG(sbuf.st_mode);
}

/** isdir
 *    Returns TRUE if given file path exists and is a directory
 */
int isdir(char *path)
{
    struct stat sbuf;

    return (stat(path, &sbuf) == 0) && S_ISDIR(sbuf.st_mode);
}

/** copyfile
 *    copies file at src to a new file dest. If dest exists, its contents will be truncated and replaced.
 */
int copyfile(char *src, char *dest)
{
    FILE *to, *from;
    char c;

    if((from = fopen(src, "r")) == NULL)
        return -1;

    if((to = fopen(dest, "w")) == NULL)
    {
        fclose(from);
        return -2;
    }

    while(!feof(from))
    {
        c = fgetc(from);
        if(ferror(from))
        {
            fclose(from);
            fclose(to);
            unlink(dest);
            return -3;
        }
        if(!feof(from))
            fputc(c, to);

        if(ferror(to))
        {
            fclose(from);
            fclose(to);
            unlink(dest);
            return -4;
        }
    }

    fclose(from);
    fclose(to);

    return 0;
}

/** linked list functions **/

/** list_prepend
 *    Allocates a new list node, attaches it to the beginning of list and sets the
 *    node data pointer to data.
 *    Returns - the new list node.
 */
list_node_t *list_prepend(list_t *list, void *data)
{
    list_node_t *new_node,
                *first_node;

    if(list_empty(*list))
    {
        (*list) = malloc(sizeof(list_node_t));
        (*list)->data = data;
        (*list)->prev = NULL;
        (*list)->next = NULL;
        return *list;
    }

    // create new node and prepend it to the list
    first_node = *list;
    new_node = malloc(sizeof(list_node_t));
    first_node->prev = new_node;
    *list = new_node;

    // set members in new node and return it
    new_node->data = data;
    new_node->prev = NULL;
    new_node->next = first_node;

    return new_node;
}

/** list_append
 *    Allocates a new list node, attaches it to the end of list and sets the
 *    node data pointer to data.
 *    Returns - the new list node.
 */
list_node_t *list_append(list_t *list, void *data)
{
    list_node_t *new_node,
                *last_node;

    if(list_empty(*list))
    {
        (*list) = malloc(sizeof(list_node_t));
        (*list)->data = data;
        (*list)->prev = NULL;
        (*list)->next = NULL;
        return *list;
    }

    // find end of list
    last_node = *list;
    while(last_node->next != NULL)
        last_node = last_node->next;

    // create new node and return it
    last_node->next = new_node = malloc(sizeof(list_node_t));
    new_node->data = data;
    new_node->prev = last_node;
    new_node->next = NULL;

    return new_node;
}

/** list_node_delete
 *    Deallocates and removes given node from the given list. It is up to the
 *    user to free any memory allocated for the node data before calling this
 *    function. Also, it is assumed that node is an element of list.
 */
void list_node_delete(list_t *list, list_node_t *node)
{
    if(node == NULL || *list == NULL) return;

    if(node->prev != NULL)
        node->prev->next = node->next;
    else
        *list = node->next; // node is first node, update list pointer

    if(node->next != NULL)
        node->next->prev = node->prev;

    free(node);
}

/** list_delete
 *    Deallocates and removes all nodes from the given list. It is up to the
 *    user to free any memory allocated for all node data before calling this
 *    function.
 */
void list_delete(list_t *list)
{
    list_node_t *prev = NULL,
                *curr = NULL;

    // delete all list nodes in the list
    list_foreach(*list, curr)
    {
        if(prev != NULL)
            free(prev);

        // if we're on the last node, delete it
        if(curr->next == NULL)
            free(curr);
        else
            prev = curr;
    }

    *list = NULL;
}

/** list_node_move_front
 *    Moves the given node to the first position of list. It is assumed that
 *    node is an element of list.
 */
void list_node_move_front(list_t *list, list_node_t *node)
{
    list_node_t *tmp;

    if(node == NULL ||
       *list == NULL ||
       node == *list)
        return;

    tmp = *list;
    node->prev->next = node->next;
    if(node->next != NULL)
        node->next->prev = node->prev;
    node->prev = NULL;
    node->next = *list;
    (*list)->prev = node;
    *list = node;
}

/** list_node_move_back
 *    Moves the given node to the last position of list. It is assumed that
 *    node is an element of list.
 */
void list_node_move_back(list_t *list, list_node_t *node)
{
    list_node_t *tmp;

    tmp = list_last_node(*list);

    if(node == NULL ||
       *list == NULL ||
       node == tmp)
        return;

    node->next->prev = node->prev;
    if(node->prev != NULL)
        node->prev->next = node->next;
    else
        *list = node->next; // first node is being moved, update list pointer
    tmp->next = node;
    node->prev = tmp;
    node->next = NULL;
}

/** list_nth_node_data
 *    Returns the nth node in list. If n is out of range, NULL is returned.
 */
void *list_nth_node_data(list_t list, int n)
{
    list_node_t *curr = NULL;

    list_foreach(list, curr)
    {
        if(n-- == 0)
            break;
    }

    return curr != NULL ? curr->data : curr;
}

/** list_first_node
 *    Returns the first node in list.
 */
list_node_t *list_first_node(list_t list)
{
    return list;
}

/** list_first_data
 *    Returns the data pointer of the first node in list.
 */
void *list_first_data(list_t list)
{
    if(list) return list->data;
    return NULL;
}

/** list_last_node
 *    Returns the last node in list.
 */
list_node_t *list_last_node(list_t list)
{
    if(list != NULL)
    {
        while(list->next != NULL)
            list = list->next;
    }

    return list;
}

/** list_last_data
 *    Returns the data pointer of the last node in list.
 */
void *list_last_data(list_t list)
{
    list_node_t *node = list_last_node(list);
    if(node) return node->data;
    return NULL;
}

/** list_empty
 *    Returns 1 if list is empty, else 0.
 */
int inline list_empty(list_t list)
{
    return list == NULL;
}

/** list_length
 *    Returns the number of elements in list
 */
int list_length(list_t list)
{
    int len = 0;
    list_node_t *curr;

    list_foreach(list, curr)
    {
        len++;
    }

    return len;
}

/** list_find_node
 *    Searches the given list for a node whose data pointer matches the given data pointer.
 *    If found, returns a pointer to the node, else, returns NULL.
 */
list_node_t *list_find_node(list_t list, void *data)
{
    list_node_t *node = NULL;

    list_foreach(list, node)
        if(node->data == data)
            break;

    return node;
}

