/* =======================================================================================

	network.c
	by orbitaldecay

	KNOWN BUGS THAT NEED FIXING:

	When playing multiplayer, all of the controllers must be enabled in the plugin
	if input is being received over the net.  Haven't found an easy way of fixing
	this yet.

   =======================================================================================
*/ 

#include "main.h"
#include "../r4300/r4300.h"
#include "../opengl/osd.h"
#include "plugin.h"
#include "SDL_net.h"
#include "network.h"

void netAddEvent(unsigned short type, int controller, DWORD value, unsigned short timer);
int netGetNextEvent(unsigned short *type, int *controller, DWORD *value, unsigned short *timer);
void netPopEvent();
void netProcessEventQueue();
void netInitEventQueue();
void netKillEventQueue() ;

int serverStart(unsigned short port);
void serverStop();
void serverStopListening();
void serverProcessMessages();
void serverAcceptConnection();
void serverBootPlayer(int n);

int clientRecvMessage(NetMessage *msg);
int clientSendMessage(NetMessage *msg);
void clientProcessMessages();
int clientConnect(char *server, int port);
void clientDisconnect();
void netProcessMessages();

/* =======================================================================================

	Various globals

   =======================================================================================
*/ 

static u_int16_t	SyncCounter = 0;	// Track VI in order to syncrhonize button events over network
static TCPsocket	clientSocket;		// Socket descriptor for connection to server
static SDLNet_SocketSet	clientSocketSet;	// Set for client connection to server
static NetEvent		*NetEventQueue = NULL;	// Pointer to queue of upcoming button events
static FILE		*netLog = NULL;
static TCPsocket	serverSocket;		// Socket descriptor for server (if this is one)
static SDLNet_SocketSet	serverSocketSet;	// Set of all connected clients, along with the server socket descriptor
static int		netDelay = 0;

NetPlayer		Player[MAX_CLIENTS];

static unsigned char	bServerIsActive = 0;	// Is the server active?
static unsigned char    bWaitForPlayers = 0;
static unsigned char	bNetplayEnabled = 0;
static unsigned char 	bClientIsConnected = 0;	// Are we connected to a server?

unsigned int gettimeofday_msec(void)
{
    struct timeval tv;
    unsigned int foo;
    
    gettimeofday(&tv, NULL);
    foo = ((tv.tv_sec % 1000000) * 1000) + (tv.tv_usec / 1000);
    return foo;
}

/* =======================================================================================

	A few getters and setters

   =======================================================================================
*/ 


unsigned short  getNetDelay() {return netDelay;}
FILE *		getNetLog() {return netLog;}
u_int16_t	getSyncCounter() {return SyncCounter;}
void		setSyncCounter(u_int16_t v) {SyncCounter = v;}
unsigned short  incSyncCounter() {SyncCounter++;}
DWORD		getNetKeys(int control) {return Player[control].keys.Value;}
void		setNetKeys(int control, DWORD value) {Player[control].keys.Value = value;}

unsigned short	clientIsConnected() {return bClientIsConnected;}
unsigned short	serverIsActive() {return bServerIsActive;}
unsigned short  serverWaitingForPlayers() {return bWaitForPlayers;}
unsigned short  netplayEnabled() {return bNetplayEnabled;}

/* =======================================================================================

	Initialize!

   =======================================================================================
*/ 

void netInitialize() {
  int n = 0, start_server, hostport;
  char hostname[20];

  FILE *netConfig;
  netLog = fopen("netlog.txt", "w");
  fprintf(netLog, "Begining net log...\n");

  netConfig = fopen("mupennet.conf", "r");

  if (!netConfig) { // If we failed to open the configuration file, quit.
     fprintf(netLog, "Failed to open mupennet.conf configuration file.\nGoodbye.");
     return;
  } 

  fscanf(netConfig, "server: %d\nhost: %s\nport: %d\n", &start_server, &hostname, &hostport);
  fclose(netConfig);

  fprintf(netLog, "Start_server %d\nHostname %s\nHostport %d\n", start_server, hostname, hostport, netDelay);
  memset(&Player, 0, sizeof(Player));
  if (SDLNet_Init() < 0) fprintf(netLog, "Failure to initialize SDLNet!\n");

  // If server started locally, always connect!
  if (start_server) {
    serverStart(SERVER_PORT);
    n = clientConnect("localhost", 7000);
  }
  else n = clientConnect(hostname, hostport);

  if (n) {
	netInitEventQueue();
	bNetplayEnabled = 1;
  }
  else {
    fprintf(netLog, "Client failed to connect to a server, playing offline.\n");
    netShutdown();
  }

}

void netShutdown() {
  clientDisconnect();
  serverStop();
  netKillEventQueue();
  fprintf(netLog, "Goodbye.\n");
  fclose(netLog);
}

/* =======================================================================================

	netInteruptLoop() : called every time there is a vi, causes emu to hang
                            similar to rompause=1 when waiting server to start.

   =======================================================================================
*/

void netInteruptLoop() {
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = 5000000;

	    if (serverWaitingForPlayers()) {
              fprintf(netLog, "waiting for signal to begin...\n");

              while (serverWaitingForPlayers()) {
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
            SyncCounter = 0;
            }
            else {
                    incSyncCounter();
                    if (serverIsActive()) serverProcessMessages();
                    if (clientIsConnected()) {
                         clientProcessMessages();
                         netProcessEventQueue();
                    }
            }
}

/* =======================================================================================

	Server routines

   =======================================================================================
*/ 

void serverStop() {
  int n;
  fprintf(netLog, "serverStop() called.\n");
  for (n = 0; n < MAX_CLIENTS; n++) if (Player[n].isConnected) serverBootPlayer(n);
  SDLNet_TCP_Close(serverSocket);
  SDLNet_FreeSocketSet(serverSocketSet);
  bServerIsActive = 0;
}

void serverStopWaitingForPlayers() {
	SDLNet_TCP_Close(serverSocket);
	SDLNet_TCP_DelSocket(serverSocketSet, serverSocket);
}

int serverStart(unsigned short port) {
        IPaddress serverAddr;

	fprintf(netLog, "serverStart() called.\n");
	if (serverIsActive()) serverStop();

	serverSocketSet = SDLNet_AllocSocketSet(MAX_CLIENTS + 1);
	SDLNet_ResolveHost(&serverAddr, NULL, port);
	
	if (serverSocket = SDLNet_TCP_Open(&serverAddr)) {
		SDLNet_TCP_AddSocket(serverSocketSet, serverSocket);
		fprintf(netLog, "Server successfully initialized on port %d.\n", port);
                bServerIsActive = 1;
		bWaitForPlayers = 1;
	}
	else fprintf(netLog, "Failed to initialize server on port %d.\n", port);
	return bServerIsActive;
}

int serverBroadcastMessage(NetMessage *msg) {
  int n;
  for (n = 0; n < MAX_CLIENTS; n++) if (Player[n].isConnected) {
	if (SDLNet_TCP_Send(Player[n].socket, msg, sizeof(NetMessage)) != sizeof(NetMessage)) {
		serverBootPlayer(n);                   // If the player disconnected, clean up.
	}
  }
}

void serverBootPlayer(int n) {
	NetMessage msg;
	msg.type = NETMSG_PLAYERQUIT;
	msg.genEvent.controller = n;

	SDLNet_TCP_Close(Player[n].socket);
	SDLNet_TCP_DelSocket(serverSocketSet, Player[n].socket);
        Player[n].isConnected = FALSE;
	fprintf(netLog, "Client %d disconnected.\n", n);
	serverBroadcastMessage(&msg);
}

void serverAcceptConnection() {
  TCPsocket newClient;
  NetMessage msg;

  int n;
  SDLNet_CheckSockets(serverSocketSet, 0);
  if (SDLNet_SocketReady(serverSocket)) {
    if (newClient = SDLNet_TCP_Accept(serverSocket)) {
        for (n = 0; n < MAX_CLIENTS; n++)
          if (!Player[n].isConnected) {
            SDLNet_TCP_AddSocket(serverSocketSet, (Player[n].socket = newClient));
            fprintf(netLog, "New connection accepted; Client %d\n", n);
            Player[n].isConnected = TRUE;
            if (n > 0) {  // Don't bother pinging player 1
		fprintf(netLog, "Sending ping.\n");
                msg.type = NETMSG_PING;
                msg.genEvent.type = NETMSG_PING;
                msg.genEvent.value = gettimeofday_msec();
                SDLNet_TCP_Send(newClient, &msg, sizeof(NetMessage));
            }
            break;
          }
        if (n == MAX_CLIENTS) SDLNet_TCP_Close(newClient); // No open slots
    }
  }
}

void serverProcessMessages() {
  NetMessage incomingMsg;
  int tempReturn, n;

  SDLNet_CheckSockets(serverSocketSet, 0);

  for (n = 0; n < MAX_CLIENTS; n++) {
    if ((Player[n].isConnected) && (SDLNet_SocketReady(Player[n].socket))) {
      tempReturn = SDLNet_TCP_Recv(Player[n].socket, &incomingMsg, sizeof(NetMessage));

      if (tempReturn <= 0) {                                     
        serverBootPlayer(n);                                     // If there was an error or the player disconnected then cleanup.
      }
      else {                   
        switch (incomingMsg.type) {
            case NETMSG_EVENT:                                   // Events (Button presses, other time sensitive things)
              if (incomingMsg.genEvent.type == NETMSG_BUTTON) {
                    incomingMsg.genEvent.timer += netDelay;      // Add calculated delay to the timer
                    if (n < 4) {                                 // If the message comes from a player (n >= 4 is for spectators)
                        incomingMsg.genEvent.controller = n;     // Change the controller tag on the event
                        serverBroadcastMessage(&incomingMsg);    // Broadcast the button event
                    }
              } else {
                    fprintf(netLog, "Server: Received unrecognized event from player %d.\n", n+1);
              }
              break;
            case NETMSG_PING:
                Player[n].lag = (gettimeofday_msec() - incomingMsg.genEvent.value);
                fprintf(netLog, "Server: Ping returned from player %d, lag %d ms.\n", n+1, Player[n].lag);
                break;
        }
      }
    }
  }
}

void serverBroadcastStart() {
  int tc = 0, ptr = 0, n, lc;
  NetMessage startmsg;
  struct timespec ts;
  short sort_array[MAX_CLIENTS];


  serverStopWaitingForPlayers();
  fprintf(getNetLog(), "Server: F9 pressed\n");
  for (n = 0; n < MAX_CLIENTS; n++) sort_array[n] = -1;

  // Sort out connection lag from highest to lowest
  for (n = 0; n < MAX_CLIENTS; n++)
      if  ( (Player[n].lag >= Player[tc].lag)
         && (Player[n].isConnected) ) 
            tc = n;

  sort_array[ptr] = tc;
  lc = tc;
  tc = 0;

  for (ptr = 1; ptr < MAX_CLIENTS; ptr++) {
     for (n = 0; n < MAX_CLIENTS; n++) 
          if ( (Player[n].lag >= Player[tc].lag)
            && (Player[n].lag <= Player[lc].lag) 
            && (n != lc) 
            && (Player[n].isConnected))
               tc = n;
     if (tc == lc) break;
     sort_array[ptr] = tc;
     lc = tc;
     tc = 0;
  }

  fprintf(netLog, "Client Lag(ms):\n");
  for (n = 0; n < MAX_CLIENTS; n++) 
        if (Player[n].isConnected) fprintf(netLog,"   Player %d: %d ms\n", sort_array[n]+1, Player[sort_array[n]].lag);

  startmsg.type = NETMSG_STARTEMU;

  netDelay = (Player[sort_array[0]].lag / 17) + 7; // 60 VI/s = ~17ms per VI (+ 7 to be safe)
  fprintf(netLog, "Net Delay: %d\n", netDelay);

  // Send STARTEMU signals, in order of slowest to fastest connection, compensate for net lag
  for (n = 0; n < MAX_CLIENTS; n++) {
     ptr = sort_array[n];
     lc  = sort_array[n+1];
     if (ptr >= 0) {
       fprintf(netLog, "Sending start message to Player %d (ping %d ms)\n", ptr + 1, Player[ptr].lag);
       SDLNet_TCP_Send(Player[ptr].socket, &startmsg, sizeof(NetMessage));
       if (lc >= 0) {
         ts.tv_sec = 0;
         ts.tv_nsec = 1000000 * (Player[ptr].lag - Player[lc].lag) / 2; // Divide by 2 for one way trip
         fprintf(netLog, "Sleeping for %d ms\n", (Player[ptr].lag - Player[lc].lag) / 2);
         nanosleep(&ts, NULL);
       }
     }
  }
  fprintf(netLog, "Finished sending STARTEMU messages.\n");
}

/* =======================================================================================

	Client routines

   =======================================================================================
*/ 

int clientConnect(char *server, int port) {
	IPaddress serverAddr;
	int n;

	fprintf(netLog, "clientConnect() called.\n");
	SDLNet_ResolveHost(&serverAddr, server, port);
	if (clientSocket = SDLNet_TCP_Open(&serverAddr)) {
		clientSocketSet = SDLNet_AllocSocketSet(1);
		SDLNet_TCP_AddSocket(clientSocketSet, clientSocket);
		bClientIsConnected = 1;
		fprintf(netLog, "Client successfully connected to %s:%d.\n", server, port);
		bWaitForPlayers = 1;
	} else fprintf(netLog, "Client failed to connected to %s:%d.\n", server, port);
	return bClientIsConnected;
}

void clientDisconnect() {
	fprintf(netLog, "clientDisconnect() called.\n");
	SDLNet_FreeSocketSet(clientSocketSet);
	SDLNet_TCP_Close(clientSocket);
	bClientIsConnected = 0;
}

int clientSendMessage(NetMessage *msg) {
  if (msg) {
    if (clientIsConnected()) {
	if (SDLNet_TCP_Send(clientSocket, msg, sizeof(NetMessage)) != sizeof(NetMessage))
	  clientDisconnect();
    }
  }
}

void clientProcessMessages() {
	NetMessage incomingMessage;
	char announceString[64];
	int n, pn;

        if (!clientIsConnected()) return; // exit now if the client isnt' connected
        SDLNet_CheckSockets(clientSocketSet, 0);
        if (!SDLNet_SocketReady(clientSocket)) return; // exit now if there aren't any messages to fetch.

	if ((n = SDLNet_TCP_Recv(clientSocket, &incomingMessage, sizeof(NetMessage))) == sizeof(NetMessage)) {
		switch (incomingMessage.type) {
			case NETMSG_EVENT:
				if (incomingMessage.genEvent.timer > getSyncCounter()) {
					netAddEvent(incomingMessage.genEvent.type, incomingMessage.genEvent.controller,
						  incomingMessage.genEvent.value,
						  incomingMessage.genEvent.timer);
                                fprintf(netLog, "Client: Event received %d %d (%d)\n", incomingMessage.genEvent.type,
                                                                               incomingMessage.genEvent.timer,
                                                                               getSyncCounter());
				}
				else fprintf(netLog, "Desync Warning!: Event received for %d, current %d\n", 						incomingMessage.genEvent.timer, getSyncCounter());
			break;
                        case NETMSG_STARTEMU:
                                fprintf(netLog, "Client: STARTEMU message received.\n");
                                bWaitForPlayers = 0;
                        break;
			case NETMSG_PLAYERQUIT:
				pn = incomingMessage.genEvent.controller;
				fprintf(netLog, "Client: Player quit announcement %d\n", pn);
				sprintf(announceString, "%s has disconnected.", Player[pn].nick);
				osd_new_message(OSD_BOTTOM_LEFT, tr(announceString));
			break;
			case NETMSG_PING:
                                fprintf(netLog, "Client: Ping received.  Returning.\n", incomingMessage.genEvent.value);
                                clientSendMessage(&incomingMessage);
			break;
			case NETMSG_SYNC:
                                fprintf(netLog, "Client: Sync packet received (%d)\n", incomingMessage.genEvent.value);
                                setSyncCounter(incomingMessage.genEvent.value);
			break;
			default:
				fprintf(netLog, "Client: Message type error.  Dropping packet.\n");
			break;

		}
	} else fprintf(netLog, "Client: Message size error (%d expecting %d).\n", n, sizeof(NetMessage));

}

void clientSendButtons(int control, DWORD value) {
  NetMessage msg;
  msg.type = NETMSG_EVENT;
  msg.genEvent.type = NETMSG_BUTTON;
  msg.genEvent.controller = control;
  msg.genEvent.value = value;
  msg.genEvent.timer = getSyncCounter();
  fprintf(netLog, "Client: Sending button state (sync %d).\n", getSyncCounter());
  clientSendMessage(&msg);
}

/* =======================================================================================

	The following functions are used to process the button event queue 

   =======================================================================================
*/ 

// netProcessEventQueue() : Process the events in the queue, if necessary.
void netProcessEventQueue() {
  int			controller, queueNotEmpty;
  DWORD			value;
  unsigned short	timer;
  unsigned short	type;
  
  if (NetEventQueue) {
    queueNotEmpty = getNextEvent(&type, &controller, &value, &timer);
    while ((timer == SyncCounter) && (queueNotEmpty)) {
	switch (type) {
          case NETMSG_BUTTON:     
            Player[controller].keys.Value = value;
            break;
       }
       netPopEvent();
       queueNotEmpty = getNextEvent(&type, &controller, &value, &timer);

    }
  }
}

// netAddEvent() : Add a new button event to the button event queue.
void netAddEvent(unsigned short type, int controller, DWORD value, unsigned short timer) {
  NetEvent *newEvent, *currEvent;

  newEvent = malloc(sizeof(NetEvent)); // TODO: Check for fail, even if it is unlikely
  newEvent->type = type;
  newEvent->controller = controller;
  newEvent->value = value;
  newEvent->timer = timer;
  newEvent->next = NULL;

 // TODO: Make sure queue is in order (lowest timer to highest timer) the packets may arrive out of order
  if (NetEventQueue) {
	currEvent = NetEventQueue;
	while(currEvent->next) {currEvent = currEvent->next;}
	currEvent->next = newEvent;
  }
  else {
	NetEventQueue = newEvent;
  }
}

// netPopEvent() : Remove the button event in the front of the queue.
void netPopEvent() {
  NetEvent *temp = NetEventQueue;

  if (NetEventQueue) {
	NetEventQueue = NetEventQueue->next;
	free(temp);
  }
}

// getNextEvent() : Retrieve information about the button event in the front of the queue.
int getNextEvent(unsigned short *type, int *controller, DWORD *value, unsigned short *timer) {
  int retValue = 1;
  NetEvent *currEvent = NetEventQueue;

  if (NetEventQueue) {
	while (NetEventQueue->timer < getSyncCounter()) {
		fprintf(netLog, "Desync Warning!: Event queue out of date (%d curr %d)! Popping next.\n",
			NetEventQueue->timer, getSyncCounter());
                netPopEvent();
	}
        *type = NetEventQueue->type;
	*controller = NetEventQueue->controller;
	*value = NetEventQueue->value;
	*timer = NetEventQueue->timer;
  }
  else {
    retValue = 0;
  }
  return retValue;
}

void netInitEventQueue() {
  SyncCounter = 0;
  while (NetEventQueue) netPopEvent();
  fprintf(netLog, "Event queue initialized.\n");
}

void netKillEventQueue() {
  while (NetEventQueue) netPopEvent();
  fprintf(netLog, "Event queue killed.\n");
}
  

