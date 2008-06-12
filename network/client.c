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
	SDLNet_ResolveHost(&serverAddr, server, port);
	if (Client.socket = SDLNet_TCP_Open(&serverAddr)) {
		Client.socketSet = SDLNet_AllocSocketSet(1);
		SDLNet_TCP_AddSocket(Client.socketSet, Client.socket);
		Client.isConnected = 1;
		fprintf((FILE *)getNetLog(), "Client successfully connected to %s:%d.\n", server, port);
		Client.isWaitingForServer = 1;
	} else fprintf((FILE *)getNetLog(), "Client failed to connected to %s:%d.\n", server, port);
	return Client.isConnected;
}

void clientDisconnect() {
	fprintf((FILE *)getNetLog(), "Client: clientDisconnect() called.\n");
	SDLNet_FreeSocketSet(Client.socketSet);
	SDLNet_TCP_Close(Client.socket);
	Client.isConnected = 0;
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
	NetMessage incomingMessage;
	char announceString[64];
	int n, pn;

        if (!clientIsConnected()) return; // exit now if the client isnt' connected
        SDLNet_CheckSockets(Client.socketSet, 0);
        if (!SDLNet_SocketReady(Client.socket)) return; // exit now if there aren't any messages to fetch.
        n = SDLNet_TCP_Recv(Client.socket, &incomingMessage, sizeof(NetMessage));

        if (n <= 0) {
          osd_new_message(OSD_BOTTOM_LEFT, (void *)tr("You've been disconnected from the server."));
          netShutdown();
          return;
        }

	if (n == sizeof(NetMessage)) {
		switch (incomingMessage.type) {
			case NETMSG_EVENT:
				if (incomingMessage.genEvent.timer > getEventCounter()) {
					addEventToQueue(incomingMessage.genEvent.type, incomingMessage.genEvent.controller,
						  incomingMessage.genEvent.value,
						  incomingMessage.genEvent.timer);
                                fprintf((FILE *)getNetLog(), "Client: Event received %d %d (%d)\n", incomingMessage.genEvent.type,
                                                                               incomingMessage.genEvent.timer,
                                                                               getEventCounter());
				}
				else fprintf((FILE *)getNetLog(), "Desync Warning!: Event received for %d, current %d\n", 						incomingMessage.genEvent.timer, getEventCounter());
			break;
                        case NETMSG_STARTEMU:
                                fprintf((FILE *)getNetLog(), "Client: STARTEMU message received.\n");
                                Client.isWaitingForServer = 0;
                        break;
			case NETMSG_PLAYERQUIT:
				pn = incomingMessage.genEvent.controller;
				fprintf((FILE *)getNetLog(), "Client: Player quit announcement %d\n", pn);
				sprintf(announceString, "Player %d has disconnected.", pn+1);
				osd_new_message(OSD_BOTTOM_LEFT, (void *)tr(announceString));
			break;
			case NETMSG_PING:
                                fprintf((FILE *)getNetLog(), "Client: Ping received.  Returning.\n", incomingMessage.genEvent.value);
                                clientSendMessage(&incomingMessage);
			break;
			case NETMSG_SYNC:
                                fprintf((FILE *)getNetLog(), "Client: Sync packet received (%d)\n", incomingMessage.genEvent.value);
                                setEventCounter(incomingMessage.genEvent.value);
			break;
			default:
				fprintf((FILE *)getNetLog(), "Client: Message type error.  Dropping packet.\n");
			break;

		}
	} else fprintf((FILE *)getNetLog(), "Client: Message size error (%d expecting %d).\n", n, sizeof(NetMessage));

}

void clientSendButtons(int control, DWORD value) {
  NetMessage msg;
  msg.type = NETMSG_EVENT;
  msg.genEvent.type = NETMSG_BUTTON;
  msg.genEvent.controller = control;
  msg.genEvent.value = value;
  msg.genEvent.timer = getEventCounter();
  fprintf((FILE *)getNetLog(), "Client: Sending button state (sync %d).\n", getEventCounter());
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
          case NETMSG_BUTTON:     
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

  if (Client.eventQueue) {
	while (Client.eventQueue->timer < getEventCounter()) {
		fprintf((FILE *)getNetLog(), "Desync Warning!: Event queue out of date (%d curr %d)! Popping next.\n",
			Client.eventQueue->timer, getEventCounter());
                popEventQueue();
	}
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
  fprintf((FILE *)getNetLog(), "Event queue initialized.\n");
}

void killEventQueue() {
  while (Client.eventQueue) popEventQueue();
  fprintf((FILE *)getNetLog(), "Event queue killed.\n");
}
  

