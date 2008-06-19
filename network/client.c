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

int clientConnect(MupenClient *Client, char *server, int port) {
	IPaddress serverAddr;

        if (Client->isConnected) clientDisconnect(Client);
        flushEventQueue(Client);
        memset(Client, 0, sizeof(Client));
	SDLNet_ResolveHost(&serverAddr, server, port);
        Client->syncFreq = 7;

	if (Client->socket = SDLNet_TCP_Open(&serverAddr)) {
		Client->socketSet = SDLNet_AllocSocketSet(1);
		SDLNet_TCP_AddSocket(Client->socketSet, Client->socket);
		Client->isConnected = 1;
		Client->isWaitingForServer = 1;
	}
	return Client->isConnected;
}

void clientDisconnect(MupenClient *Client) {
       if (Client->isConnected) {
          printf("[Netplay] Disconnected from server.\n");
          osd_new_message(OSD_BOTTOM_LEFT, (void *)tr("You have disconnected from the server."));
          SDLNet_FreeSocketSet(Client->socketSet);
          SDLNet_TCP_Close(Client->socket);
          Client->isConnected = 0;
          flushEventQueue(Client);
       }
}

int clientSendMessage(MupenClient *Client, NetMessage *msg) {
  if (msg) {
    if (Client->isConnected) {
	if (SDLNet_TCP_Send(Client->socket, msg, sizeof(NetMessage)) != sizeof(NetMessage)) // If there is a problem sending then disconnect
	  clientDisconnect(Client);
    }
  }
}

void clientProcessMessages(MupenClient *Client) {
	NetMessage incomingMessage, outgoingMessage;
	char osdString[64];
	int n, playerNumber;

        if (!Client->isConnected) return; // exit now if the client isnt' connected
        SDLNet_CheckSockets(Client->socketSet, 0);
        while (SDLNet_SocketReady(Client->socket)) {
          n = SDLNet_TCP_Recv(Client->socket, &incomingMessage, sizeof(NetMessage));
          if (n <= 0) {
            clientDisconnect(Client);
            return;
          }
          if (n == sizeof(NetMessage)) {
                playerNumber = incomingMessage.genEvent.player;
		switch (incomingMessage.type) {
			case NETMSG_EVENT:
				if (incomingMessage.genEvent.timer >= Client->frameCounter) {
					addEventToQueue(Client, incomingMessage);
				}
				else {
                                   printf("[Netplay] DESYNC: Recieved a late event for frame %d (currently frame %d).\n", incomingMessage.genEvent.timer, Client->frameCounter);
                                   outgoingMessage.type = NETMSG_DESYNC;
                                   clientSendMessage(Client, &outgoingMessage);
                                }
			break;
                        case NETMSG_DESYNC:
				sprintf(osdString, "Player %d has desynchronized!", playerNumber + 1);
                                printf("[Netplay] %s\n", osdString);
				osd_new_message(OSD_BOTTOM_LEFT, (void *)tr(osdString));
                        break;
                        case NETMSG_SYNC:
                                //printf("[Netplay] Recieved sync message (End of frame %d)\n", incomingMessage.genEvent.timer);
				Client->lastSync = incomingMessage.genEvent.timer;
                        break;
			case NETMSG_PLAYERQUIT:
				sprintf(osdString, "Player %d has disconnected.", playerNumber + 1);
                                printf("[Netplay] %s\n", osdString);
				osd_new_message(OSD_BOTTOM_LEFT, (void *)tr(osdString));
			break;
			case NETMSG_PING:
                                clientSendMessage(Client, &incomingMessage);
                                printf("[Netplay] Received ping, responding.\n");
			break;
			default:
                                printf("[Netplay] Received an unrecognized message from the server.\n");
			break;
		}
	  }
        }
}

void clientSendButtons(MupenClient *Client, int control, DWORD value) {
  NetMessage msg;
  msg.type = NETMSG_EVENT;
  msg.genEvent.type = EVENT_BUTTON;
  msg.genEvent.player = control;
  msg.genEvent.value = value;
  msg.genEvent.timer = 0;	
  clientSendMessage(Client, &msg);
}

/* =======================================================================================

	The following functions are used to process the event queue 

   =======================================================================================
*/ 

// processEventQueue() : Process the events in the queue, if necessary.
void processEventQueue(MupenClient *Client) {
  NetMessage outgoingMessage;

  if (!Client->isConnected) return; // exit now if the client isnt' connected
  if (Client->eventQueue) {
    while (Client->eventQueue->timer < Client->frameCounter) {
       printf("[Netplay] DESYNC: Missed event for frame %d (currently frame %d).\n", Client->eventQueue->timer, Client->frameCounter);
       outgoingMessage.type = NETMSG_DESYNC;
       clientSendMessage(Client, &outgoingMessage);
       popEventQueue(Client);
    }
    while ((Client->eventQueue) && (Client->eventQueue->timer == Client->frameCounter)) {
	switch (Client->eventQueue->type) {
          case EVENT_BUTTON: 
            if (Client->eventQueue->controller < MAX_CLIENTS) {
                Client->playerKeys[Client->eventQueue->controller].Value = Client->eventQueue->value;
            } else {
                printf("[Netplay] Received message from player %d (out of range).\n", Client->eventQueue->controller);
            }
            break;
       }
       popEventQueue(Client);
    }
  }
}

// addEventToQueue() : Add a new button event to the button event queue.
void addEventToQueue(MupenClient *Client, NetMessage msg) {
  NetEvent *newEvent, *currEvent;

  newEvent = malloc(sizeof(NetEvent)); // TODO: Check for fail, even if it is unlikely
  newEvent->type = msg.genEvent.type;
  newEvent->controller = msg.genEvent.player;
  newEvent->value = msg.genEvent.value;
  newEvent->timer = msg.genEvent.timer;
  newEvent->next = NULL;

 // TODO: Make sure queue is in order (lowest timer to highest timer) the packets may arrive out of order
  if (Client->eventQueue) {
	currEvent = Client->eventQueue;
	while(currEvent->next) {currEvent = currEvent->next;}
	currEvent->next = newEvent;
  }
  else {
	Client->eventQueue = newEvent;
  }
}

// popEventQueue() : Remove the event in the front of the queue.
void popEventQueue(MupenClient *Client) {
  NetEvent *temp = Client->eventQueue;

  if (Client->eventQueue) {
	Client->eventQueue = Client->eventQueue->next;
        free(temp);
  }
}


void flushEventQueue(MupenClient *Client) {
  while (Client->eventQueue) popEventQueue(Client);
}

  

