/* =======================================================================================

	network.c
	by orbitaldecay

	Current problems (bugs that need fixing):

	When playing multiplayer, all of the controllers must be enabled in the plugin
	if input is being received over the net.  Haven't found an easy way of fixing
	this yet.

        All clients must be using the same core.

        Obviously all clients must use the same rom, the ms doesn't check yet.

   =======================================================================================
*/ 


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
  SDLNet_Init();
  fprintf(stderr,"netInitialize()\n");
  rompause=1;
  return clientInitialize(mClient);
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
    int sentSyncMessage = 0;
    struct timespec ts;
    if (mClient->numConnected>0 && (mClient->frameCounter % VI_PER_FRAME)==0) {
        int i;
        for(i=0; i<mClient->numConnected-1;i++){
            int curID=sourceID(mClient->myID, i);
            mClient->packet->address=mClient->player[curID].address;
            mClient->packet->len=4;
            SDLNet_Write16((mClient->frameCounter/VI_PER_FRAME)&FRAME_MASK,mClient->packet->data);
            mClient->packet->data[2]=mClient->myID;
            mClient->packet->data[3]=mClient->lag[curID];
            fprintf(stderr,"send sync %08x\n",mClient->packet->address.host);
            SDLNet_UDP_Send(mClient->socket, -1, mClient->packet);
        }
    }
    clientProcessMessages(mClient);
    processEventQueue(mClient);
    mClient->frameCounter++;
    mClient->frameCounter=mClient->frameCounter&FRAME_MASK;

    return 0;

            //else fprintf(stderr,"Frame %d, bool %d",mClient->frameCounter,mClient->numConnected);
		/*
                if (mClient->frameCounter % mClient->syncFreq == 0) mClient->isWaitingForServer = TRUE;
                if (mClient->isWaitingForServer) {
                    sentSyncMessage = 0;
                    while (mClient->isWaitingForServer && mClient->isConnected) {
                        osd_render();  // Updating OSD
                        SDL_GL_SwapBuffers();
                        SDL_PumpEvents();
#ifdef WITH_LIRC
                        lircCheckInput();
#endif //WITH_LIRC 
                        if (mServer->isActive) {
                            if (mServer->isAccepting) {
                                serverAccept(mServer);
                                serverProcessMessages(mServer);
                            } else if (!sentSyncMessage) {
                                serverProcessMessages(mServer);
                                syncMsg.type = NETMSG_SYNC;
                                syncMsg.genEvent.timer = mClient->frameCounter;
                                serverBroadcastMessage(mServer, &syncMsg);
                                sentSyncMessage = 1;
                            }
                        }
                        clientProcessMessages(mClient);
                        processEventQueue(mClient);
                        if (mClient->lastSync >= mClient->frameCounter) mClient->isWaitingForServer = FALSE;
                    }
	        } else {
                  //if (mServer->isActive) serverProcessMessages(mServer);
                  clientProcessMessages(mClient);
                  processEventQueue(mClient);
                }
            }*/

}

int frameDelta(MupenClient *Client, Uint32 frame) {
  int result=Client->frameCounter-frame;
  if (result > (FRAME_MASK*VI_PER_FRAME)/2)
    result-=result-FRAME_MASK*VI_PER_FRAME;
  else if (-result > (FRAME_MASK*VI_PER_FRAME)/2)
    result+=FRAME_MASK*VI_PER_FRAME;
  return result;
}

int sourceID(int myID, int index) {
    if(index<myID)
        return index;
    return index+1;
}
