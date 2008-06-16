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


static FILE		*netLog = NULL;

static unsigned char	bNetplayEnabled = 0;

FILE *		getNetLog() {return netLog;}
unsigned short  netplayEnabled() {return bNetplayEnabled;}

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

        to be called when program starts


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

void netStartNetplay(MupenServer *mupenServer) {
  int              n = 0;
  NetPlaySettings  netSettings;
  FILE            *netConfig;

  net_init(mupenServer);  //Make sure log file is open to avoid crashes

  // Right now we're reading the netplay settings from a conf file, soon this will be part of the GUI
  // ===============================================================================================
  netConfig = fopen("mupennet.conf", "r");
  if (!netConfig) {
     netSettings.runServer = 1;
  } else {
     fscanf(netConfig, "server: %d\nhost: %s\nport: %d\n", &netSettings.runServer, &netSettings.hostname, &netSettings.port);
     fclose(netConfig);
  }
  // ===============================================================================================
  // Right now we're reading the netplay settings from a conf file, soon this will be part of the GUI

  if (netSettings.runServer) {
      msStart(mupenServer, SERVER_PORT);
      strcpy(netSettings.hostname, "localhost"); // If we're hosting a game, connect to it.
      netSettings.port = SERVER_PORT;
  }

  if (clientConnect(netSettings.hostname, netSettings.port))
      bNetplayEnabled = 1;
  else {
      netShutdown(mupenServer);
  }

}
/* ======================================================================================

      netShutdown()

========================================================================================= */

void netShutdown(MupenServer *mupenServer) {
  if (clientIsConnected()) clientDisconnect();
  if (msIsActive(mupenServer)) msStop(mupenServer);
  bNetplayEnabled = 0;
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

                    if (msIsActive(mupenServer) && msIsAccepting(mupenServer)) {
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

            if (msIsActive(mupenServer)) msProcessMessages(mupenServer);
            if (clientIsConnected()) {
                clientProcessMessages();
                processEventQueue();
            }

            if ((getEventCounter() % SYNC_FREQ == 0) && (clientIsConnected())) {
                if (msIsActive(mupenServer)) {
                    syncMsg.type = NETMSG_SYNC;
                    syncMsg.genEvent.timer = getEventCounter();
                    msBroadcastMessage(mupenServer, &syncMsg);
                }
                else {
                    if (clientLastSyncMsg() < getEventCounter()) {
                        clientPauseForServer();
                    } else if (clientLastSyncMsg() > getEventCounter()) {
                    } else {
                    }
                }
            }
            incEventCounter();
}







