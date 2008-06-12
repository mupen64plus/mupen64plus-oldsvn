/* =======================================================================================

	network.c
	by orbitaldecay

	Current problems (bugs that need fixing):

	When playing multiplayer, all of the controllers must be enabled in the plugin
	if input is being received over the net.  Haven't found an easy way of fixing
	this yet.

        All clients must be using the same core.

        Obviously all clients must use the same rom, the server doesn't check yet.

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

      netInitialize()



========================================================================================= */

void netInitialize() {
  int n = 0, start_server = 0, hostport = 0;
  char hostname[20] = "";
  FILE *netConfig;

  netLog = fopen("netlog.txt", "w");
  fprintf(netLog, "Begining net log...\n");
  netConfig = fopen("mupennet.conf", "r");
  if (!netConfig) {
     fprintf(netLog, "Failed to open mupennet.conf configuration file.\n");
     start_server = 1;
  } else {
     fscanf(netConfig, "server: %d\nhost: %s\nport: %d\n", &start_server, &hostname, &hostport);
     fclose(netConfig);
  }

  fprintf(netLog, "Start_server %d\nHostname %s\nHostport %d\n", start_server, hostname, hostport);

  if (SDLNet_Init() < 0) {
     fprintf(netLog, "Failure to initialize SDLNet!\n");
     return;
  }

  clientInitialize();
  serverInitialize();
  if (start_server) {
      serverStart(SERVER_PORT);
      n = clientConnect("localhost", 7000);
  }
  else n = clientConnect(hostname, hostport);

  if (n) {
      initEventQueue();
      bNetplayEnabled = 1;
  }
  else {
      fprintf(netLog, "Client failed to connect to a server, playing offline.\n");
      netShutdown();
  }

}
/* ======================================================================================

      netShutdown()



========================================================================================= */

void netShutdown() {
  if (clientIsConnected()) {
    clientDisconnect();
    killEventQueue();
  }
  if (serverIsActive()) serverStop();
  bNetplayEnabled = 0;

  fprintf(netLog, "Goodbye.\n");
  fclose(netLog);
}

/* ======================================================================================

      netInteruptLoop()

          This function is called each time a vertical interupt is issued.

========================================================================================= */

void netInteruptLoop() {
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = 5000000;

	    if (clientWaitingForServer()) {
              fprintf(netLog, "waiting for signal to begin...\n");

              while (clientWaitingForServer()) {
                    osd_render();  // Continue updating OSD
                    SDL_GL_SwapBuffers();
                    SDL_PumpEvents();
#ifdef WITH_LIRC
                    lircCheckInput();
#endif //WITH_LIRC
                    if (clientIsConnected()) {
			clientProcessMessages();
		    }
                    if (serverIsActive()) {
			serverAcceptConnection();
			serverProcessMessages();
		    }
                    nanosleep(&ts, NULL); // sleep for 5 milliseconds so it doesn't rail the processor
	      }
              setEventCounter(0);
            }
            else {
                    incEventCounter();
                    if (serverIsActive()) serverProcessMessages();
                    if (clientIsConnected()) {
                         clientProcessMessages();
                         processEventQueue();
                    }
            }
}







