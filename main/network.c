/* =======================================================================================

	network.c
	by orbitaldecay

	Alright, so far the most basic network functionality has been included.  It still
	needs to be incorporated in the gui, there are no ROM checks to ensure the clients
	are running the same rom, and various other essential features are lacking; but
	what is here should be enough to enable netplay.  In the running directory of
	mupen64plus is a file titled mupennet.conf.  The first number represents whether
	or not to start the server, the second word is the hostname to connect to and the
	final number is the port to connect on.  For example:

	1 localhost 7000

	Run the server and connect to the localhost on port 7000.

	0 192.168.0.152 7000

	Don't run a server, connect to 192.168.0.152 on port 7000.

	You get the idea.  Once the clients are connected, you must press F9 on the server
	to send a message to all the clients telling them to begin running the ROM, this is
	extremely crude and will be fixed later.  Like I say, this is the most basic functionality
	imaginable, but the code does compensate for network latency, up to 100ms pings should 
	be fine and remain perfectly synchronized.  Email me with any questions or comments
	orbitaldecay@gmail.com.  Thanks ^_^

	Bob

	KNOWN BUGS THAT NEED FIXING:

	When playing multiplayer, all of the controllers must be enabled in the plugin
	if input is being received over the net.  Haven't found an easy way of fixing
	this yet.

   =======================================================================================
*/ 

#include "main.h"
#include "../r4300/r4300.h"
#include "plugin.h"
#include "SDL_net.h"
#include "../opengl/osd.h"
#include "network.h"

/* =======================================================================================

	Globals for use within network.c
	TODO: Actually define scope, don't be lazy!

   =======================================================================================
*/ 

static BUTTONS		netKeys[4];		// Key cache
static unsigned short	netVISyncCounter = 0;	// Track VI in order to syncrhonize button events over network

static TCPsocket	clientSocket;		// Socket descriptor for connection to server
static SDLNet_SocketSet	clientSocketSet;	// Set for client connection to server
static unsigned char 	bClientIsConnected = 0;	// Are we connected to a server?

static NetButtonEvent	*netButtonEventQueue = NULL;	// Pointer to queue of upcoming button events
static FILE		*netLog = NULL;

static pthread_t        serverThread;
static TCPsocket	serverSocket;		// Socket descriptor for server (if this is one)
static SDLNet_SocketSet	serverSocketSet;	// Set of all connected clients, along with the server socket descriptor

static unsigned char	bServerIsActive = 0;	// Is the server active?
static unsigned char    bServerStop = 0;	// Do we need the server to stop?
static unsigned char    bEmulatorIsRunning = 0;




static TCPsocket	Client[MAX_CLIENTS];
static char PlayerName[20][MAX_CLIENTS] = {"Player 1", "Player 2", "Player 3", "Player 4"};



/* =======================================================================================

	Initialize!

   =======================================================================================
*/ 

void netInitialize() {
  int n = 0, start_server, hostport;
  char hostname[20];

  FILE *netConfig;
  netLog = fopen("netlog.txt", "w");

  netConfig = fopen("mupennet.conf", "r");
  fscanf(netConfig, "%d %s %d\n", &start_server, &hostname, &hostport);
  fclose(netConfig);

  fprintf(netLog, "Begining net log...\n");

  fprintf(netLog, "Start_server %d\nHostname %s\nHostport %d\n", start_server, hostname, hostport);
  for (n = 0; n < 4; n++) netKeys[n].Value = 0;
  for (n = 0; n < MAX_CLIENTS; n++) Client[n] = 0;
  if (SDLNet_Init() < 0) fprintf(netLog, "Failure to initialize SDLNet!\n");

  // If server started locally, always connect!
  if (start_server) {
    serverStart(SERVER_PORT);
    n = clientConnect("localhost", 7000);
  }
  else n = clientConnect(hostname, hostport);

  if (n) netInitButtonQueue();
  else {
    fprintf(netLog, "Client failed to connect to a server, playing offline.\n");
    netShutdown();
  }

}

void netShutdown() {
  clientDisconnect();
  serverStop();
  netKillButtonQueue();
  bEmulatorIsRunning = 0;
  fprintf(netLog, "Goodbye.\n");
  fclose(netLog);
}

/* =======================================================================================

	Access to assorted variables outside of network.c

   =======================================================================================
*/ 


FILE *		netGetLog() {return netLog;}
unsigned short	netGetSyncCounter() {return netVISyncCounter;}
void		netSetSyncCounter(unsigned short v) {netVISyncCounter = v;}
DWORD		netGetKeys(int control) {return netKeys[control].Value;}
void		netSetKeys(int control, DWORD value) {netKeys[control].Value = value;}

unsigned short	netClientIsConnected() {return bClientIsConnected;}
unsigned short	netServerIsActive() {return bServerIsActive;}

/* =======================================================================================

	netMain: do the regular mainloop stuff

   =======================================================================================
*/

// netMain() : Handle everything!
void netMain() {
  if (netServerIsActive()) serverProcessMessages();
  if (netClientIsConnected()) {
	netProcessButtonQueue();
	netClientProcessMessages();
  }
}


/* =======================================================================================

	Server routines

   =======================================================================================
*/ 

void serverStop() {
  int n;
  fprintf(netLog, "serverStop() called.\n");
  for (n = 0; n < MAX_CLIENTS; n++) if (Client[n]) serverKillClient(n);
  SDLNet_TCP_Close(serverSocket);
  SDLNet_FreeSocketSet(serverSocketSet);
  bServerIsActive = 0;
}

void serverStopListening() {
	SDLNet_TCP_Close(serverSocket);
	SDLNet_TCP_DelSocket(serverSocketSet, serverSocket);
}

int serverStart(unsigned short port) {
        IPaddress serverAddr;

	fprintf(netLog, "serverStart() called.\n");
	if (netServerIsActive()) serverStop();

	serverSocketSet = SDLNet_AllocSocketSet(MAX_CLIENTS + 1);
	SDLNet_ResolveHost(&serverAddr, NULL, port);
	
	if (serverSocket = SDLNet_TCP_Open(&serverAddr)) {
		SDLNet_TCP_AddSocket(serverSocketSet, serverSocket);
		//serverThread = pthread_create(&serverThread, NULL, serverLoop, NULL);
		fprintf(netLog, "Server successfully initialized on port %d.\n", port);
                bServerIsActive = 1;
	}
	else fprintf(netLog, "Failed to initialize server on port %d.\n", port);
	return bServerIsActive;
}

int serverRecvMessage(TCPsocket client, NetMessage *msg) {
	int netRet = 0;
	SDLNet_CheckSockets(serverSocketSet, 0);
	if (SDLNet_SocketReady(client))
		netRet = SDLNet_TCP_Recv(client, msg, sizeof(NetMessage));
	return netRet;
}


int serverSendMessage(TCPsocket client, NetMessage *msg) {
  return SDLNet_TCP_Send(client, msg, sizeof(NetMessage));
}

int serverBroadcastMessage(NetMessage *msg) {
  int n;
  for (n = 0; n < MAX_CLIENTS; n++) if (Client[n]) {
	if (serverSendMessage(Client[n], msg) != sizeof(NetMessage)) {
		serverKillClient(n);
	}
  }
}

void serverKillClient(int n) {
	NetMessage msg;
	SDLNet_TCP_Close(Client[n]);
	SDLNet_TCP_DelSocket(serverSocketSet, Client[n]);
	Client[n] = 0;
	fprintf(netLog, "Client %d disconnected.\n", n);

	msg.type = NETMSG_PLAYERQUIT;
	msg.data.joinRequest.controller = n;
	serverBroadcastMessage(&msg);
	fprintf(netLog, "Broadcast player quit.\n");
}

void serverAcceptConnection() {
  TCPsocket newClient;
  int n;
  SDLNet_CheckSockets(serverSocketSet, 0);
  if (SDLNet_SocketReady(serverSocket)) {
    if (newClient = SDLNet_TCP_Accept(serverSocket)) {
        for (n = 0; n < MAX_CLIENTS; n++)
          if (!Client[n]) {
            SDLNet_TCP_AddSocket(serverSocketSet, (Client[n] = newClient));
            fprintf(netLog, "New connection accepted; Client %d\n", n);
            break;
          }
        if (n == MAX_CLIENTS) SDLNet_TCP_Close(newClient); // No open slots
    }
  }
}

void serverProcessMessages() {
	NetMessage msg;
	int recvRet, n;

	SDLNet_CheckSockets(serverSocketSet, 5); // 5 ms wait so server loop doesn't rail incesantly
	for (n = 0; n < MAX_CLIENTS; n++) {
		if (Client[n]) {
			if (SDLNet_SocketReady(Client[n])) {
				if (recvRet = serverRecvMessage(Client[n], &msg)) {
					switch (msg.type) {
						case NETMSG_BUTTON:
							msg.data.buttonEvent.timer += netDelay;
							if (n < 4) {
								msg.data.buttonEvent.controller = n;
								serverBroadcastMessage(&msg);
							}
						break;
						case NETMSG_SETNAME:
							msg.data.joinRequest.controller = n;
							serverBroadcastMessage(&msg);
						break;
					}
				} else {
					serverKillClient(n);
				}
			}
		}
	}
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
	} else fprintf(netLog, "Client failed to connected to %s:%d.\n", server, port);
	return bClientIsConnected;
}

void clientDisconnect() {
	fprintf(netLog, "netClientDisconnect() called.\n");
	SDLNet_FreeSocketSet(clientSocketSet);
	SDLNet_TCP_Close(clientSocket);
	bClientIsConnected = 0;
}

int netClientRecvMessage(NetMessage *msg) {
	int netRet = 0;
	SDLNet_CheckSockets(clientSocketSet, 0);
	if (SDLNet_SocketReady(clientSocket))
		netRet = SDLNet_TCP_Recv(clientSocket, msg, sizeof(NetMessage));
	return netRet;
}

int netClientSendMessage(NetMessage *msg) {
  if (msg) {
    if (netClientIsConnected()) {
	if (SDLNet_TCP_Send(clientSocket, msg, sizeof(NetMessage)) != sizeof(NetMessage))
	  clientDisconnect();
    }
  }
}

void netClientProcessMessages() {
	NetMessage incomingMessage;
	char announceString[64];
	int n, pn;

	if ((n = netClientRecvMessage(&incomingMessage)) == sizeof(NetMessage)) {
		switch (incomingMessage.type) {
			case NETMSG_BUTTON:
				if (incomingMessage.data.buttonEvent.timer > netGetSyncCounter()) {
					netAddButtonEvent(incomingMessage.data.buttonEvent.controller,
						  incomingMessage.data.buttonEvent.value,
						  incomingMessage.data.buttonEvent.timer);
				}
				else fprintf(netLog, "Desync Warning!: button event received for %d, current %d\n", 						incomingMessage.data.buttonEvent.timer, netGetSyncCounter());
			break;
			case NETMSG_STARTEMU:
				rompause = 0;
				fprintf(netLog, "Client STARTEMU message received.  Lets go!\n");
				bEmulatorIsRunning = 1;
			break;
			case NETMSG_PLAYERQUIT:
				pn = incomingMessage.data.joinRequest.controller;
				fprintf(netLog, "Player quit announcement %d\n", pn);
				sprintf(announceString, "%s has disconnected.", PlayerName[pn]);
				osd_new_message(OSD_BOTTOM_LEFT, tr(announceString));
			break;
			case NETMSG_SETNAME:
				pn = incomingMessage.data.joinRequest.controller;
				sprintf(announceString, "%s has changed their name to %s.", PlayerName[pn], 						incomingMessage.data.joinRequest.nickname);
				strcpy(PlayerName[pn], incomingMessage.data.joinRequest.nickname);
				osd_new_message(OSD_BOTTOM_LEFT, tr(announceString));
			break;
			default:
				fprintf(netLog, "Client message type error.  Dropping packet.\n");
			break;

		}
	} else if (n != 0) fprintf(netLog, "Client message size error.  Dropping packet.\n"); // Check for server disconnects!
}

void netSendButtonState(int control, DWORD value) {
  NetMessage msg;
  msg.type = NETMSG_BUTTON;
  msg.data.buttonEvent.controller = control;
  msg.data.buttonEvent.value = value;
  msg.data.buttonEvent.timer = netGetSyncCounter();
  netClientSendMessage(&msg);
}

/* =======================================================================================

	The following functions are used to process the button event queue 

   =======================================================================================
*/ 

// netProcessButtonQueue() : Process the events in the queue, if necessary.
void netProcessButtonQueue() {
  int			controller, queueNotEmpty;
  DWORD			value;
  unsigned short	timer;
  
  netVISyncCounter++;
  if (netButtonEventQueue) {
    queueNotEmpty = netGetNextButtonEvent(&controller, &value, &timer);
    while ((timer == netVISyncCounter) && (queueNotEmpty)) {
	  netKeys[controller].Value = value;
  	  netPopButtonEvent();
          queueNotEmpty = netGetNextButtonEvent(&controller, &value, &timer);
    }
  }
}

// netAddButtonEvent() : Add a new button event to the button event queue.
void netAddButtonEvent(int controller, DWORD value, unsigned short timer) {
  NetButtonEvent *newButtonEvent, *currButtonEvent;

  newButtonEvent = malloc(sizeof(NetButtonEvent)); // TODO: Check for fail, even if it is unlikely
  newButtonEvent->controller = controller;
  newButtonEvent->value = value;
  newButtonEvent->timer = timer;
  newButtonEvent->next = NULL;

 // TODO: Make sure queue is in order (lowest timer to highest timer) the packets may arrive out of order
  if (netButtonEventQueue) {
	currButtonEvent = netButtonEventQueue;
	while(currButtonEvent->next) {currButtonEvent = currButtonEvent->next;}
	currButtonEvent->next = newButtonEvent;
  }
  else {
	netButtonEventQueue = newButtonEvent;
  }
}

// netPopButtonEvent() : Remove the button event in the front of the queue.
void netPopButtonEvent() {
  NetButtonEvent *temp = netButtonEventQueue;

  if (netButtonEventQueue) {
	netButtonEventQueue = netButtonEventQueue->next;
	free(temp);
  }
}

// netGetNextButtonEvent() : Retrieve information about the button event in the front of the queue.
int netGetNextButtonEvent(int *controller, DWORD *value, unsigned short *timer) {
  int retValue = 1;
  NetButtonEvent *currButtonEvent = netButtonEventQueue;

  if (netButtonEventQueue) {
	while (currButtonEvent->timer < netGetSyncCounter()) {
		netPopButtonEvent();
		fprintf(netLog, "Desync Warning!: Button queue out of date (%d curr %d)! Popping next.\n",
			currButtonEvent->timer, netGetSyncCounter());
	}
	*controller = netButtonEventQueue->controller;
	*value = netButtonEventQueue->value;
	*timer = netButtonEventQueue->timer;
  }
  else {
    retValue = 0;
  }
  return retValue;
}

void netInitButtonQueue() {
  netVISyncCounter = 0;
  while (netButtonEventQueue) netPopButtonEvent();
  fprintf(netLog, "Button event queue initialized.\n");
}

void netKillButtonQueue() {
  while (netButtonEventQueue) netPopButtonEvent();
  fprintf(netLog, "Button event queue killed.\n");
}
  

