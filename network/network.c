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


unsigned int gettimeofday_msec(void)
{
    struct timeval tv;
    unsigned int foo;
    
    gettimeofday(&tv, NULL);
    foo = ((tv.tv_sec % 1000000) * 1000) + (tv.tv_usec / 1000);
    return foo;
}

int netStartNetplay(MupenServer *mServer, MupenClient *mClient, NetPlaySettings netSettings) {
  int ret = 0;

  SDLNet_Init();
  if (netSettings.runServer) {
      serverStart(mServer, SERVER_PORT);
      strcpy(netSettings.hostname, "localhost"); // If we're hosting a game, connect to it.
  }
  if (clientConnect(mClient, netSettings.hostname, SERVER_PORT)) {
      printf("[Netplay] Connected to %s.\n", netSettings.hostname);
      ret = 1;
  }
  return ret;
}

void netShutdown(MupenServer *mServer, MupenClient *mClient) {
  clientDisconnect(mClient);
  serverStop(mServer);
}

int netMain(MupenServer *mServer, MupenClient *mClient) {
            int sentSyncMessage = 0;
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = 5000000;
            NetMessage syncMsg;

            if (mClient->isConnected) {
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
                  if (mServer->isActive) serverProcessMessages(mServer);
                  clientProcessMessages(mClient);
                  processEventQueue(mClient);
                }
            }

            mClient->frameCounter++;
            return 0;
}







