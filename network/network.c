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
    if (mClient->numConnected>0 && (mClient->frameCounter % VI_PER_FRAME)==0)
        clientSendFrame(mClient);
    clientProcessMessages(mClient);
    processEventQueue(mClient);
    mClient->frameCounter++;

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
