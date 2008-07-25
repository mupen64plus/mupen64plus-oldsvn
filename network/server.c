/* =======================================================================================

	server.c
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

u_int16_t getEventCounter() {
    MupenClient *mClient;
    mClient = (MupenClient *)getNetplayClient();
    return mClient->frameCounter;
}

u_int16_t getNextSync() {
    MupenClient *mClient;
    mClient = (MupenClient *)getNetplayClient();
    return mClient->lastSync + mClient->syncFreq;
}

u_int16_t getSyncFreq() {
    MupenClient *mClient;
    mClient = (MupenClient *)getNetplayClient();
    return mClient->syncFreq;
}

int serverStart(MupenServer *Server, unsigned short port) {
        IPaddress msAddr;

	if (Server->isActive) return;
        memset(Server, 0, sizeof(MupenServer));
        Server->netDelay = 3;  // For now --should be calculated (5 is approximate)
	Server->socketSet = SDLNet_AllocSocketSet(MAX_CLIENTS + 1);
	SDLNet_ResolveHost(&msAddr, NULL, port);
	
	if (Server->socket = SDLNet_TCP_Open(&msAddr)) {
		SDLNet_TCP_AddSocket(Server->socketSet, Server->socket);
                Server->isActive = 1;
		Server->isAccepting = 1;
	}
	return Server->isActive;
}

void serverStop(MupenServer *Server) {
  int n;
  if (Server->isActive) {
      for (n = 0; n < MAX_CLIENTS; n++) if (Server->player[n].isConnected) serverBootPlayer(Server, n);
      SDLNet_FreeSocketSet(Server->socketSet);
      if (Server->isAccepting) {
        SDLNet_TCP_Close(Server->socket);
        Server->isAccepting = 0;
      }
      Server->isActive = 0;
  }
}

void serverStopWaitingForPlayers(MupenServer *Server) {
	SDLNet_TCP_DelSocket(Server->socketSet, Server->socket);
        Server->isAccepting = 0;
}

int serverBroadcastMessage(MupenServer *Server, NetMessage *msg) {
  int n;
  for (n = 0; n < MAX_CLIENTS; n++) if (Server->player[n].isConnected) {
	if (SDLNet_TCP_Send(Server->player[n].socket, msg, sizeof(NetMessage)) != sizeof(NetMessage)) {
		serverBootPlayer(Server, n);                   // If the player disconnected, clean up.
	}
  }
}

void serverBootPlayer(MupenServer *Server, int n) {
	NetMessage msg;
	msg.type = NETMSG_PLAYERQUIT;
	msg.genEvent.player = n;

	if (Server->player[n].isConnected) {
          printf("Player %d disconnected.\n", n+1);
          SDLNet_TCP_Close(Server->player[n].socket);
          SDLNet_TCP_DelSocket(Server->socketSet, Server->player[n].socket);
          Server->player[n].isConnected = FALSE;
          serverBroadcastMessage(Server, &msg);
        }
}

void serverAccept(MupenServer *Server) {
  TCPsocket newClient;
  NetMessage msg;

  int n, allPlayersReady = 1;

  SDLNet_CheckSockets(Server->socketSet, 0);
  if (SDLNet_SocketReady(Server->socket)) {
    if (newClient = SDLNet_TCP_Accept(Server->socket)) {
        for (n = 0; n < MAX_CLIENTS; n++)
          if (!Server->player[n].isConnected) {
            printf("Player %d has connected.\n", n+1);
            SDLNet_TCP_AddSocket(Server->socketSet, (Server->player[n].socket = newClient));
            Server->player[n].isConnected = TRUE;
            msg.type = NETMSG_PING;
            msg.genEvent.type = NETMSG_PING;
            msg.genEvent.value = gettimeofday_msec();
            SDLNet_TCP_Send(newClient, &msg, sizeof(NetMessage));
            break;
          }
        if (n == MAX_CLIENTS) SDLNet_TCP_Close(newClient); // No open slots
    }
  }

  for (n = 0; n < MAX_CLIENTS; n++) {
    if ((Server->player[n].isConnected) && (!Server->player[n].isReady)) allPlayersReady = 0;
  }

  if (allPlayersReady) {
    printf("All players ready, begining game.\n");
    serverStopWaitingForPlayers(Server);
    msg.type = NETMSG_SYNC;
    msg.genEvent.timer = getEventCounter();
    serverBroadcastMessage(Server, &msg);    
  } 
 
}

void serverProcessMessages(MupenServer *Server) {
  NetMessage incomingMsg;
  int tempReturn, n;
  struct timespec ts;

  ts.tv_sec = 0;

  SDLNet_CheckSockets(Server->socketSet, 0);

  for (n = 0; n < MAX_CLIENTS; n++) {
    while ((Server->player[n].isConnected) && (SDLNet_SocketReady(Server->player[n].socket))) {
      tempReturn = SDLNet_TCP_Recv(Server->player[n].socket, &incomingMsg, sizeof(NetMessage));

      if (tempReturn <= 0) {
        serverBootPlayer(Server, n);                                     // If there was an error or the player disconnected then cleanup.
      } else {                   
        switch (incomingMsg.type) {
            case NETMSG_EVENT:                                           // Events (Button presses, other time sensitive input)
              if (incomingMsg.genEvent.type == EVENT_BUTTON) {
                    incomingMsg.genEvent.timer = getEventCounter() + getSyncFreq();          // Can't desync this way
                    if (n < 4) {                                         // If the message comes from a player (n >= 4 is for spectators)
                        incomingMsg.genEvent.player = n;                 // Change the controller tag on the event
                        serverBroadcastMessage(Server, &incomingMsg);    // Broadcast the button event
                    }
              }
              break;
            case NETMSG_READY: // Client is notifying server that it is ready to begin
                Server->player[n].isReady = TRUE;
                printf("Player %d is ready.\n", n+1);
                break;
            case NETMSG_PING: // Client is returning a ping request
                Server->player[n].lag = (gettimeofday_msec() - incomingMsg.genEvent.value);
                break;
            case NETMSG_DESYNC: // Client is notifying the server that it has desynchronized
                printf("Player %d has desynchronized.\n", n+1);
                incomingMsg.genEvent.player = n;
                serverBroadcastMessage(Server, &incomingMsg);
                break;
            default:
                printf("Unrecognized message received from player %d.\n", n+1);
                break;
        }
      }
    }
  }
}

