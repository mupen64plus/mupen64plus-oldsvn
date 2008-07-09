/*
################################################################################
================================================================================

 * Mupen64Plus - master.c
 * Copyright (C) 2008 orbitaldecay
 *
 * Mupen64Plus homepage: http://code.google.com/p/mupen64plus/
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

  Functions for contacting the master server

================================================================================
################################################################################
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL_net.h>
#include <pthread.h>
#include "network.h"
#include "../main/main.h"

#define STATUS_SUBSYSTEM_ERROR -1
#define STATUS_NO_RESPONSE     -2
#define STATUS_OK               0
#define MASTER_SERVER_TIMEOUT   500

IPaddress            g_Game_Master;
MupenClient          g_NetplayClient;

static HostListNode *l_MasterServerList = NULL;
static int           l_Game_ID = -1;
static pthread_t     l_Keep_Alive_Thread = 0;   

void MasterServerAddToList(char *arg) {
  static HostListNode *last_node = NULL;
  IPaddress     server_addy;
  char          host_buffer[256], master_server[512], *ptr;
  uint16_t      host_port;
  int           n;

  // Copy to buffer in case arg is a string literal
  strncpy(master_server, arg, sizeof(master_server));

  // Replace ':' with ' ' for scanf compatability
  ptr = strchr(master_server, ':');
  if (ptr == NULL) {
     printf("[Master Server] Address format not recognized for \"%s\".\n", master_server);
     return;
  }
  n = (int)(ptr - master_server);
  if (n < sizeof(host_buffer)) master_server[n] = 32;
  else {
     printf("[Master Server] Hostname \"%s\" is too long.\n", master_server);
     return;
  }

  // Extract hostname and port
  sscanf(master_server, "%s %u", host_buffer, &host_port);
  if (SDLNet_ResolveHost(&server_addy, host_buffer, host_port) == -1) {
      printf("[Master Server] Unable to resolve hostname \"%s\".\n", host_buffer);
      return;
  }

  printf("[Master Server] Adding %s:%u to master server list.\n", host_buffer, host_port);
  if (!l_MasterServerList) {
     l_MasterServerList = malloc(sizeof(HostListNode));
     if (!l_MasterServerList) {
        printf("malloc(): Failure.  Unable to allocate memory.\n");
        FreeHostList(l_MasterServerList);
        return;
     }
     l_MasterServerList->host = server_addy.host;
     l_MasterServerList->port = server_addy.port;
     last_node = l_MasterServerList;
  } else {
     last_node->next = malloc(sizeof(HostListNode));
     if (!last_node->next) {
        printf("malloc(): Failure.  Unable to allocate memory.\n");
        FreeHostList(l_MasterServerList);
        return;
     }
     last_node->next->host = server_addy.host;
     last_node->next->port = server_addy.port;
     last_node = last_node->next;
  }
  last_node->next = NULL;
}

void MasterServerCloseGame() {
  l_Game_ID = -1;
}

static void *KeepAliveThread( void *_arg ) {
  printf("[Master Server] Keep alive thread launched.\n");

  while(l_Game_ID != -1) {
    masterServerKeepAlive(g_Game_Master.host, g_Game_Master.port, l_Game_ID);
    sleep(5);
  }

  printf("[Master Server] Exiting keep alive thread.\n");
}

int MasterServerCreateGame(unsigned char md5[16], int local_port) {
    HostListNode *temp_master;

    if (l_Game_ID != -1) {
       // Kill old thread
       l_Game_ID = -1;
    }

    temp_master = GetFirstMasterServer();
    while ((l_Game_ID == -1) && (temp_master)) {
        printf("[Master Server] Opening game @ %d.%d.%d.%d:%d...", GET_IP(temp_master->host), GET_PORT(temp_master->port));
        l_Game_ID = masterServerOpenGame(temp_master->host, temp_master->port, md5, local_port);
        if (l_Game_ID != -1) {
           printf("Game ID: %d\n", l_Game_ID);
           g_Game_Master.host = temp_master->host;
           g_Game_Master.port = temp_master->port;

           // Launch keep alive thread
           if(pthread_create(&l_Keep_Alive_Thread, NULL, KeepAliveThread, NULL) != 0)
           {
               l_Keep_Alive_Thread = 0;
               alert_message(tr("[Master Server] Couldn't spawn keep alive thread!"));
               l_Game_ID = -1;
           }
        } else {
           printf("No response.\n");
        }
        temp_master = GetNextHost(temp_master);
    }
    return l_Game_ID;
}

HostListNode *MasterServerFindGames(unsigned char md5[16]) {
    int status = STATUS_NO_RESPONSE;
    HostListNode *host_list = NULL;
    HostListNode *temp_master;

    temp_master = GetFirstMasterServer();
    while (temp_master) {
        printf("[Master Server] Finding games @ %d.%d.%d.%d:%d...", GET_IP(temp_master->host), GET_PORT(temp_master->port));
        host_list = CombineHostLists(host_list, masterServerFindGames(temp_master->host, temp_master->port, md5, &status));
        switch (status) {
            case STATUS_SUBSYSTEM_ERROR:
                 printf("Error\n");
                 break;
            case STATUS_NO_RESPONSE:
                 printf("No Response\n");
                 break;
            case STATUS_OK:
                 printf("Ok\n");
                 break;        
        }
        temp_master = GetNextHost(temp_master);
    }
    return host_list;
}

MD5ListNode *MasterServerGetMD5List() {
    int status = STATUS_NO_RESPONSE;
    MD5ListNode  *MD5_list = NULL;
    HostListNode *temp_master;

    temp_master = GetFirstMasterServer();
    while (temp_master) {
        printf("[Master Server] Retreiving MD5 list from %d.%d.%d.%d:%d...", GET_IP(temp_master->host), GET_PORT(temp_master->port));
        MD5_list = CombineMD5Lists(MD5_list, masterServerGetMD5List(temp_master->host, temp_master->port, &status));
        switch (status) {
            case STATUS_SUBSYSTEM_ERROR:
                 printf("Error\n");
                 break;
            case STATUS_NO_RESPONSE:
                 printf("No Response\n");
                 break;
            case STATUS_OK:
                 printf("Ok\n");
                 break;        
        }
        temp_master = GetNextHost(temp_master);
    }
    return MD5_list;
}

void FreeMD5List(MD5ListNode *list) {
    MD5ListNode *temp = list;
    while (temp) {
        list = list->next;
        free(temp);
        temp = list;
    }
}

void FreeHostList(HostListNode *list) {
    HostListNode *temp = list;
    while (temp) {
        list = list->next;
        free(temp);
        temp = list;
    }
}

HostListNode *GetFirstMasterServer() {
  return l_MasterServerList;
}

HostListNode *GetNextHost(HostListNode *node) {
  return node->next;
}


HostListNode *CombineHostLists(HostListNode *list1, HostListNode *list2) {
  int duplicate = 0;
  HostListNode *temp1 = list1;
  HostListNode *temp2 = list2;

  if (!temp1) return temp2;

  while (temp2) {
      duplicate = 0;
      if ((temp1->host == temp2->host) && (temp1->port == temp2->port)) duplicate = 1;
      while ((GetNextHost(temp1)) && (!duplicate)) {
          temp1 = GetNextHost(temp1);
          if ((temp1->host == temp2->host) && (temp1->port == temp2->port)) duplicate = 1;
      }
      if (!duplicate) temp1->next = temp2;
      temp1 = list1;
      temp2 = GetNextHost(temp2);
  }
  return list1;
}

MD5ListNode *CombineMD5Lists(MD5ListNode *list1, MD5ListNode *list2) {
  int duplicate = 0;
  MD5ListNode *temp1 = list1;
  MD5ListNode *temp2 = list2;

  if (!temp1) return temp2;

  while (temp2) {
      duplicate = 0;
      if (memcmp(temp1->md5, temp2->md5, 16) == 0) duplicate = 1;
      while ((GetNextMD5(temp1)) && (!duplicate)) {
          temp1 = GetNextMD5(temp1);
          if (memcmp(temp1->md5, temp2->md5, 16) == 0) duplicate = 1;
      }
      if (!duplicate) temp1->next = temp2;
      temp1 = list1;
      temp2 = GetNextMD5(temp2);
  }
  return list1;
}

MD5ListNode *GetNextMD5(MD5ListNode *node) {
  return node->next;
}


/*
################################################################################
================================================================================

  Internal Functions

================================================================================
################################################################################
*/

static long int timeElapsed(unsigned char arm) {
  static struct timeval arm_tv;
  struct timeval curr_tv;
  long int elapsed = 0;

  if (arm) {
      gettimeofday(&arm_tv, NULL);
  } else {
      gettimeofday(&curr_tv, NULL);
      elapsed = ((curr_tv.tv_sec - arm_tv.tv_sec) * 1000) + ((curr_tv.tv_usec - curr_tv.tv_usec) / 1000);
  }
  return elapsed;
}

/*
    masterServerOpenGame
    ------------------------------------------------------------------------------
*/

static int masterServerOpenGame(uint32_t master_server, uint16_t master_port, unsigned char md5[16], uint16_t local_port) {
    IPaddress    serverAddy;
    UDPpacket   *p;
    uint16_t     gameDesc;
    int          recv;

    // Allocate a packet buffer
    p = SDLNet_AllocPacket(32);
    if (!p) {
        printf("SDLNet_AllocPacket(): %s\n", SDLNet_GetError());
        return -1;
    }

    // Fill packet data
    p->data[0] = 'M';
    p->data[1] = '+';
    p->data[2] = 1;
    p->data[3] = 0;
    p->data[4] = OPEN_GAME;
    memcpy(p->data+5, md5, 16);
    SDLNet_Write16(local_port, p->data + 21);

    // Define packet length and destination
    p->len = 23;
    p->address.host = master_server;
    p->address.port = master_port;

    // We should be sending master server requests from the UDP game port
    // (i.e. the port that the client will be accepting p2p connections on)
    // This way, in the event of a firewall, we'll already have a NAT entry
    // for the master server so we can receive FRAME_PUNCHREQUESTs.
    if (!g_NetplayClient.isListening) {
        printf("[Master Server] Client not initialized (bind g_NetplayClient.socket).\n");
        return -1;
    }

    if (!SDLNet_UDP_Send(g_NetplayClient.socket, -1, p)) {
        printf("SDLNet_UDP_Send(): %s\n", SDLNet_GetError());
        return -1;
    }

    timeElapsed(1);
    while (((recv = SDLNet_UDP_Recv(g_NetplayClient.socket, p)) == 0) && (timeElapsed(0) < MASTER_SERVER_TIMEOUT)) {}

    // Handle possible errors
    if (recv == -1) {
        printf("SDLNet_UDP_Recv(): %s\n", SDLNet_GetError());
        return -1;
    } else if (recv == 0) {
        return -1;
    }

    // response received from master server, extract game id
    if (p->data[0] == GAME_DESC) gameDesc = SDLNet_Read16(p->data + 1);

    // Cleanup and return game id
    SDLNet_FreePacket(p);
    return gameDesc;
}

/*
    masterServerKeepAlive (master server, master server port, game ID)
    ------------------------------------------------------------------------------
    sends keep alive packet to master server.  returns 0 on success. -1 on error.
*/

static int masterServerKeepAlive(uint32_t master_server, uint16_t master_port, uint16_t game_id) {
    IPaddress  serverAddy;
    UDPsocket  s;
    UDPpacket *p;
    uint16_t   gameDesc;

    // Resolve address of master server
    serverAddy.host = master_server;
    serverAddy.port = master_port;

    // Allocate a packet buffer
    p = SDLNet_AllocPacket(32);
    if (!p) {
        printf("SDLNet_AllocPacket(): %s\n", SDLNet_GetError());
        return -1;
    }

    // Bind a udp socket
    s = SDLNet_UDP_Open(0);
    if (!s) {
        printf("SDLNet_UDP_Open(): %s\n", SDLNet_GetError());
        return -1;
    }

    // Fill packet data
    p->data[0] = 'M';
    p->data[1] = '+';
    p->data[2] = 1;
    p->data[3] = 0;
    p->data[4] = KEEP_ALIVE;
    SDLNet_Write16(game_id, p->data + 5);

    // Define packet length and destination
    p->len = 7;
    p->address.host = serverAddy.host;
    p->address.port = serverAddy.port;

    if (!SDLNet_UDP_Send(s, -1, p)) {
        printf("SDLNet_UDP_Send(): %s\n", SDLNet_GetError());
        return -1;
    }
    SDLNet_FreePacket(p);
    return 0;
}

/*
    masterServerFindGames
    ------------------------------------------------------------------------------
*/

static HostListNode *masterServerFindGames(uint32_t master_server, uint16_t master_port, unsigned char md5[16], int *status) {
    HostListNode *HostList = NULL, *tempList = NULL;
    IPaddress     serverAddy;
    UDPsocket     s;
    UDPpacket *   p;
    uint32_t      gameAddy;
    uint16_t      gamePort;
    int           recv, n;

    // Resolve address of master server
    serverAddy.host = master_server;
    serverAddy.port = master_port;

    // Allocate a packet buffer
    p = SDLNet_AllocPacket(1470); // Currently max length for packet from master server
    if (!p) {
        printf("SDLNet_AllocPacket(): %s\n", SDLNet_GetError());
        *status = STATUS_SUBSYSTEM_ERROR;
        return NULL;
    }

    // Bind a udp socket
    s = SDLNet_UDP_Open(0);
    if (!s) {
        printf("SDLNet_UDP_Open(): %s\n", SDLNet_GetError());
        *status = STATUS_SUBSYSTEM_ERROR;
        return NULL;
    }

    // Fill packet data
    p->data[0] = 'M';
    p->data[1] = '+';
    p->data[2] = 1;
    p->data[3] = 0;
    p->data[4] = FIND_GAMES;
    memcpy(p->data+5, md5, 16);

    // Define packet length and destination
    p->len = 21;
    p->address.host = serverAddy.host;
    p->address.port = serverAddy.port;

    if (!SDLNet_UDP_Send(s, -1, p)) {
        printf("SDLNet_UDP_Send(): %s\n", SDLNet_GetError());
        *status = STATUS_SUBSYSTEM_ERROR;
        return NULL;
    }

    do {
        timeElapsed(1);
        while ( ((recv = SDLNet_UDP_Recv(s, p)) == 0) && (timeElapsed(0) < MASTER_SERVER_TIMEOUT) ) {}

        // Subsystem error
        if (recv == -1) {
            printf("SDLNet_UDP_Recv(): %s\n", SDLNet_GetError());
           *status = STATUS_SUBSYSTEM_ERROR;
        // Timed out, no response
        } else if (recv == 0) {
           *status = STATUS_NO_RESPONSE;
        // Received game list packet, parse it and make a list out of it
        } else if (p->data[0] == GAME_LIST) {
           *status = STATUS_OK;
            for (n = 0; n < (p->len - 2) / 6; n++) {
               memcpy(&gameAddy, (p->data + (n*6+2)), 4);
               gamePort = SDLNet_Read16(p->data + (n*6+6));
               if (HostList) {
                   tempList->next = malloc(sizeof(HostListNode));
                   if (!tempList->next) {
                      printf("malloc(): Failure! Unable to allocate memory.\n");
                      FreeHostList(HostList);
                      *status = STATUS_SUBSYSTEM_ERROR;
                      return NULL;
                   }
                   tempList->next->host = gameAddy;
                   tempList->next->port = gamePort;
                   tempList = tempList->next;
               } else {
                   HostList = malloc(sizeof(HostListNode));
                   if (!HostList) {
                      printf("malloc(): Failure! Unable to allocate memory.\n");
                      FreeHostList(HostList);
                      *status = STATUS_SUBSYSTEM_ERROR;
                      return NULL;
                   }
                   HostList->host = gameAddy;
                   HostList->port = gamePort;
                   tempList = HostList;
               }
            }
        }
    } while ((p->data[1]) && (recv > 0));

    // Cleanup and return pointer to game linked list
    SDLNet_FreePacket(p);
    if (tempList) tempList->next = NULL;
    return HostList;
}

/*
    masterServerGetMD5List
    ------------------------------------------------------------------------------
*/
static MD5ListNode *masterServerGetMD5List(uint32_t master_server, uint16_t master_port, int *status) {
    MD5ListNode * MD5List = NULL, *tempList = NULL;
    IPaddress     serverAddy;
    UDPsocket     s;
    UDPpacket *   p;
    unsigned char md5[16];
    int           recv, n, m;

    // Resolve address of master server
    serverAddy.host = master_server;
    serverAddy.port = master_port;

    // Allocate a packet buffer
    p = SDLNet_AllocPacket(1470); // Currently max length for packet from master server
    if (!p) {
        printf("SDLNet_AllocPacket(): %s\n", SDLNet_GetError());
        *status = STATUS_SUBSYSTEM_ERROR;
        return NULL;
    }

    // Bind a udp socket
    s = SDLNet_UDP_Open(0);
    if (!s) {
        printf("SDLNet_UDP_Open(): %s\n", SDLNet_GetError());
        *status = STATUS_SUBSYSTEM_ERROR;
        return NULL;
    }

    // Fill packet data
    p->data[0] = 'M';
    p->data[1] = '+';
    p->data[2] = 1;
    p->data[3] = 0;
    p->data[4] = FIND_MD5;

    // Define packet length and destination
    p->len = 5;
    p->address.host = serverAddy.host;
    p->address.port = serverAddy.port;

    if (!SDLNet_UDP_Send(s, -1, p)) {
        printf("SDLNet_UDP_Send(): %s\n", SDLNet_GetError());
        *status = STATUS_SUBSYSTEM_ERROR;
        return NULL;
    }

    do {
        timeElapsed(1);
        while ( ((recv = SDLNet_UDP_Recv(s, p)) == 0) && (timeElapsed(0) < MASTER_SERVER_TIMEOUT) ) {}

        // Subsystem error
        if (recv == -1) {
            printf("SDLNet_UDP_Recv(): %s\n", SDLNet_GetError());
           *status = STATUS_SUBSYSTEM_ERROR;
        // Timed out, no response
        } else if (recv == 0) {
           *status = STATUS_NO_RESPONSE;
        // Received MD5 list packet, parse it and make a list out of it
        } else if (p->data[0] == MD5_LIST) {
           *status = STATUS_OK; 
           for (n = 0; n < (p->len - 2) / 16; n++) {
                if (MD5List) {
                   tempList->next = malloc(sizeof(MD5ListNode));
                   if (!tempList->next) {
                      printf("malloc(): Failure! Unable to allocate memory.\n");
                      FreeMD5List(MD5List);
                      *status = STATUS_SUBSYSTEM_ERROR;
                      return NULL;
                   }
                   memcpy(tempList->next->md5, (p->data + (n * 16 + 2)), 16);
                   tempList = tempList->next;
               } else {
                   MD5List = malloc(sizeof(MD5ListNode));
                   if (!MD5List) {
                      printf("malloc(): Failure! Unable to allocate memory.\n");
                      FreeMD5List(MD5List);
                      *status = STATUS_SUBSYSTEM_ERROR; 
                      return NULL;
                   }
                   memcpy(MD5List->md5, (p->data + (n * 16 + 2)), 16);
                   tempList = MD5List;
               }
            }
        }
    } while ((p->data[1]) && (recv > 0));

    // Cleanup and return pointer to game linked list
    SDLNet_FreePacket(p);
    if (tempList) tempList->next = NULL;
    return MD5List;
}
