/* =======================================================================================

	client.c
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
MupenClient     Client;

DWORD		getNetKeys(int control) {return Client.playerKeys[control].Value;}
void		setNetKeys(int control, DWORD value) {Client.playerKeys[control].Value = value;}
BOOL            clientWaitingForServer() { return Client.isWaitingForServer;}
void            clientPauseForServer() { Client.isWaitingForServer = TRUE;}
u_int16_t       clientLastSyncMsg() { return Client.lastSync; }
BOOL            clientIsConnected() {return Client.isConnected;}
u_int16_t	getEventCounter() {return Client.eventCounter;}
void		setEventCounter(u_int16_t v) {Client.eventCounter = v;}
void            incEventCounter() {Client.eventCounter++;}


void clientInitialize() {
        memset(&Client, 0, sizeof(Client));

}

int clientConnect(char *server, int port) {
	IPaddress serverAddr;
	int n;


        if (clientIsConnected()) clientDisconnect();
        initEventQueue();
        clientInitialize();
	SDLNet_ResolveHost(&serverAddr, server, port);

	if (Client.socket = SDLNet_TCP_Open(&serverAddr)) {
		Client.socketSet = SDLNet_AllocSocketSet(1);
		SDLNet_TCP_AddSocket(Client.socketSet, Client.socket);
		Client.isConnected = 1;
		Client.isWaitingForServer = 1;
	}
	return Client.isConnected;
}

void clientDisconnect() {
        if (!Client.isConnected) {
          SDLNet_FreeSocketSet(Client.socketSet);
          SDLNet_TCP_Close(Client.socket);
          Client.isConnected = 0;
          killEventQueue();
        }
}

int clientSendMessage(NetMessage *msg) {
  if (msg) {
    if (clientIsConnected()) {
	if (SDLNet_TCP_Send(Client.socket, msg, sizeof(NetMessage)) != sizeof(NetMessage))
	  clientDisconnect();
    }
  }
}

void clientProcessMessages() {
	NetMessage incomingMessage, outgoingMessage;
	char osdString[64];
	int n, playerNumber;

        if (!clientIsConnected()) return; // exit now if the client isnt' connected
        SDLNet_CheckSockets(Client.socketSet, 0);
        if (!SDLNet_SocketReady(Client.socket)) return; // exit now if there aren't any messages to fetch.
        n = SDLNet_TCP_Recv(Client.socket, &incomingMessage, sizeof(NetMessage));

        if (n <= 0) {
          printf("[Netplay] Disconnected from server.\n");
          osd_new_message(OSD_BOTTOM_LEFT, (void *)tr("You have disconnected from the server."));
          clientDisconnect();
          return;
        }

	if (n == sizeof(NetMessage)) {
                playerNumber = incomingMessage.genEvent.controller;
		switch (incomingMessage.type) {
			case NETMSG_EVENT:
				if (incomingMessage.genEvent.timer >= getEventCounter()) {
					addEventToQueue(incomingMessage.genEvent.type, incomingMessage.genEvent.controller,
						  incomingMessage.genEvent.value,
						  incomingMessage.genEvent.timer);
				}
				else {
                                   outgoingMessage.type = NETMSG_DESYNC;
                                   clientSendMessage(&outgoingMessage);
                                }
			break;
                        case NETMSG_DESYNC:
				sprintf(osdString, "Player %d has desynchronized!", playerNumber + 1);
                                printf("[Netplay] %s\n", osdString);
				osd_new_message(OSD_BOTTOM_LEFT, (void *)tr(osdString));
                        break;
                        case NETMSG_SYNC:
                                Client.isWaitingForServer = 0;
				Client.lastSync = incomingMessage.genEvent.timer;
                        break;
			case NETMSG_PLAYERQUIT:
				sprintf(osdString, "Player %d has disconnected.", playerNumber + 1);
                                printf("[Netplay] %s\n", osdString);
				osd_new_message(OSD_BOTTOM_LEFT, (void *)tr(osdString));
			break;
			case NETMSG_PING:
                                clientSendMessage(&incomingMessage);
			break;
			default:
                                printf("[Netplay] Received an unrecognized message from the server.\n");
			break;

		}
	}

}

void clientSendButtons(int control, DWORD value) {
  NetMessage msg;
  msg.type = NETMSG_EVENT;
  msg.genEvent.type = EVENT_BUTTON;
  msg.genEvent.controller = control;
  msg.genEvent.value = value;
  msg.genEvent.timer = 0;	
  clientSendMessage(&msg);
}

/* =======================================================================================

	The following functions are used to process the event queue 

   =======================================================================================
*/ 

// processEventQueue() : Process the events in the queue, if necessary.
void processEventQueue() {
  int			controller, queueNotEmpty;
  DWORD			value;
  unsigned short	timer;
  unsigned short	type;
  
  if (Client.eventQueue) {
    queueNotEmpty = getNextEvent(&type, &controller, &value, &timer);
    while ((timer == Client.eventCounter) && (queueNotEmpty)) {
	switch (type) {
          case EVENT_BUTTON:     
            setNetKeys(controller, value);
            break;
       }
       popEventQueue();
       queueNotEmpty = getNextEvent(&type, &controller, &value, &timer);

    }
  }
}

// addEventToQueue() : Add a new button event to the button event queue.
void addEventToQueue(unsigned short type, int controller, DWORD value, unsigned short timer) {
  NetEvent *newEvent, *currEvent;

  newEvent = malloc(sizeof(NetEvent)); // TODO: Check for fail, even if it is unlikely
  newEvent->type = type;
  newEvent->controller = controller;
  newEvent->value = value;
  newEvent->timer = timer;
  newEvent->next = NULL;

 // TODO: Make sure queue is in order (lowest timer to highest timer) the packets may arrive out of order
  if (Client.eventQueue) {
	currEvent = Client.eventQueue;
	while(currEvent->next) {currEvent = currEvent->next;}
	currEvent->next = newEvent;
  }
  else {
	Client.eventQueue = newEvent;
  }
}

// popEventQueue() : Remove the event in the front of the queue.
void popEventQueue() {
  NetEvent *temp = Client.eventQueue;

  if (Client.eventQueue) {
	Client.eventQueue = Client.eventQueue->next;
        free(temp);
  }
}

// getNextEvent() : Retrieve information about the event in the front of the queue.
int getNextEvent(unsigned short *type, int *controller, DWORD *value, unsigned short *timer) {
  int retValue = 1;
  NetEvent *currEvent = Client.eventQueue;

  while ((Client.eventQueue) && (Client.eventQueue->timer < getEventCounter())) {
      popEventQueue();
  }

  if (Client.eventQueue) {
      *type = Client.eventQueue->type;
      *controller = Client.eventQueue->controller;
      *value = Client.eventQueue->value;
      *timer = Client.eventQueue->timer;
  }
  else {
      retValue = 0;
  }
        
  return retValue;
}

void initEventQueue() {
  while (Client.eventQueue) popEventQueue();
}

void killEventQueue() {
  while (Client.eventQueue) popEventQueue();
}
  

