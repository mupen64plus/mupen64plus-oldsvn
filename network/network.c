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

/* ======================================================================================

      net_init()

        to be called when emu starts


========================================================================================= */

void net_init(MupenServer *mupenServer) {
  clientInitialize();
  msInitialize(mupenServer);
  setEventCounter(0);
  SDLNet_Init();
}

/* ======================================================================================

      netStartNetplay()

        to be called when rom starts


========================================================================================= */
void netReadConfigFile(NetPlaySettings *netSettings) {
  FILE            *netConfig;
  netConfig = fopen("mupennet.conf", "r");
  if (!netConfig) {
     netSettings->runServer = 1;
  } else {
     fscanf(netConfig, "server: %d\nhost: %s\nport: %d\n", &(netSettings->runServer), &(netSettings->hostname), &(netSettings->port));
     fclose(netConfig);
  }
}

int netStartNetplay(MupenServer *mupenServer, NetPlaySettings netSettings) {
  net_init(mupenServer);
  if (netSettings.runServer) {
      msStart(mupenServer, SERVER_PORT);
      strcpy(netSettings.hostname, "localhost"); // If we're hosting a game, connect to it.
      netSettings.port = SERVER_PORT;
  }

  return clientConnect(netSettings.hostname, netSettings.port);
}
/* ======================================================================================

      netShutdown()

========================================================================================= */

void netShutdown(MupenServer *mupenServer) {
  if (clientIsConnected()) clientDisconnect();
  if (mupenServer->isActive) msStop(mupenServer);
}

/* ======================================================================================

      netInteruptLoop()

          This function is called each time a vertical interupt is issued.

========================================================================================= */

void netInteruptLoop(MupenServer *mupenServer) {
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = 5000000;
            NetMessage syncMsg;

	    if (clientWaitingForServer()) {

              while (clientWaitingForServer() && clientIsConnected()) {
                    osd_render();  // Updating OSD
                    SDL_GL_SwapBuffers();
                    SDL_PumpEvents();

                    if (mupenServer->isActive && mupenServer->isAccepting) {
#ifdef WITH_LIRC
                        lircCheckInput();
#endif //WITH_LIRC 
			msAcceptConnection(mupenServer);
			msProcessMessages(mupenServer);
                        nanosleep(&ts, NULL);
                    }
                    clientProcessMessages();
	      }
            }

            if (mupenServer->isActive) msProcessMessages(mupenServer);
            if (clientIsConnected()) {
                clientProcessMessages();
                processEventQueue();
            }

            if ((getEventCounter() % SYNC_FREQ == 0) && (clientIsConnected())) {
                if (mupenServer->isActive) {
                    syncMsg.type = NETMSG_SYNC;
                    syncMsg.genEvent.timer = getEventCounter();
                    msBroadcastMessage(mupenServer, &syncMsg);
                } else if (clientLastSyncMsg() < getEventCounter()) {
                        clientPauseForServer();
                }
            }
            incEventCounter();
}







