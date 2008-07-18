/**
 * Mupen64Plus - network.c
 * 2008 orbitaldecay & DarkJeztr
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
**/


#include "network.h"
#include "../r4300/r4300.h"

unsigned int gettimeofday_msec(void)
{
    struct timeval tv;
    unsigned int foo;
    
    gettimeofday(&tv, NULL);
    foo = ((tv.tv_sec % 1000000) * 1000) + (tv.tv_usec / 1000);
    return foo;
}

int netInitialize(MupenClient *mClient) {
  int retValue;

  SDLNet_Init();
  rompause = 1;
  g_Game_Master.host = 0;
  MasterServerCloseGame();

 if ((!clientInitialize(mClient)) ||  (!netLaunchRecvThread())) {
     printf("[Netplay] Network initialization failed.\n");
     return 0;
 } else printf("[Netplay] Network initialization sucessful.\n");

 return 1;
}

int netStartNetplay(MupenClient *mClient, NetPlaySettings netSettings) {
  int ret = 0;

  if (netSettings.hostname[0]!='\0') {
    if (clientConnect(mClient, netSettings.hostname, SERVER_PORT)) {
      printf("[Netplay] Connected to %s.\n", netSettings.hostname);
      ret = 1;
    }
  }
  else
    ret = 1;
  return ret;
}

void netShutdown(MupenClient *mClient) {
    clientDisconnect(mClient);
}

int netMain(MupenClient *mClient) {
    static int lastSent;

    if (mClient->numConnected>0 && (mClient->frameCounter % VI_PER_FRAME)==0 &&
            lastSent!=mClient->frameCounter) {
        clientSendFrame(mClient);
        lastSent=mClient->frameCounter;
    }

//        Now being called from netReceiveThread() so that packets can be processed without running the core
//        netReceiveThread() is launched from netInitialize()
//        clientProcessMessages(mClient);

    if( processEventQueue(mClient) )
        mClient->frameCounter++;

    return 0;
}

static void *netReceiveThread( void *_arg ) {
  unsigned short int    counter = 0;
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 1000000; // 1 ms

  printf("[Netplay] Netplay receive thread started.\n");

  while(g_NetplayClient.socket) {
      counter++;
      if ((counter % 5000 == 0) && (g_Game_ID != -1)) {
         MasterServerKeepAlive(g_Game_Master.host, g_Game_Master.port, g_Game_ID, g_NetplayClient.socket);
      }
      clientProcessMessages(&g_NetplayClient);
      nanosleep (&ts, NULL);
  }
  printf("[Netplay] Exiting netplay receive thread.\n");
}

int netLaunchRecvThread() {
    pthread_t netRecvThread;
    if(pthread_create(&netRecvThread, NULL, netReceiveThread, NULL) != 0)
    {
       printf("[Netplay] Couldn't spawn recv thread.\n");
       return 0;
    } else {
       printf("[Netplay] Recv thread spawned: id %d\n", netRecvThread);
       return 1;
    }
}

int frameDelta(MupenClient *Client, Uint32 frame) {
  int result=Client->frameCounter-frame;
  while (result > (FRAME_MASK*VI_PER_FRAME)/2)
    result-=result-FRAME_MASK*VI_PER_FRAME;
  while (-result > (FRAME_MASK*VI_PER_FRAME)/2)
    result+=FRAME_MASK*VI_PER_FRAME;
  return result;
}

int sourceID(int myID, int index) {
    if(index<myID)
        return index;
    return index+1;
}
