/* =======================================================================================

	server.c
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

BOOL            msIsActive(MupenServer *Server) {return Server->isActive;}
BOOL            msIsAccepting(MupenServer *Server) {return Server->isAccepting;}
unsigned short  getNetDelay(MupenServer *Server) {return Server->netDelay;}


void msInitialize(MupenServer *Server) {
        memset(Server, 0, sizeof(MupenServer));
        Server->netDelay = 5;
}

int msStart(MupenServer *Server, unsigned short port) {
        IPaddress msAddr;

	if (msIsActive(Server)) return;

	Server->socketSet = SDLNet_AllocSocketSet(MAX_CLIENTS + 1);
	SDLNet_ResolveHost(&msAddr, NULL, port);
	
	if (Server->socket = SDLNet_TCP_Open(&msAddr)) {
		SDLNet_TCP_AddSocket(Server->socketSet, Server->socket);
                Server->isActive = 1;
		Server->isAccepting = 1;
	}
	return Server->isActive;
}

void msStop(MupenServer *Server) {
  int n;
  for (n = 0; n < MAX_CLIENTS; n++) if (Server->player[n].isConnected) msBootPlayer(Server, n);
  SDLNet_FreeSocketSet(Server->socketSet);
  if (Server->isAccepting) {
    SDLNet_TCP_Close(Server->socket);
    Server->isAccepting = 0;
  }
  Server->isActive = 0;
}

void msStopWaitingForPlayers(MupenServer *Server) {
	SDLNet_TCP_DelSocket(Server->socketSet, Server->socket);
        Server->isAccepting = 0;
}

int msBroadcastMessage(MupenServer *Server, NetMessage *msg) {
  int n;
  for (n = 0; n < MAX_CLIENTS; n++) if (Server->player[n].isConnected) {
	if (SDLNet_TCP_Send(Server->player[n].socket, msg, sizeof(NetMessage)) != sizeof(NetMessage)) {
		msBootPlayer(Server, n);                   // If the player disconnected, clean up.
	}
  }
}

void msBootPlayer(MupenServer *Server, int n) {
	NetMessage msg;
	msg.type = NETMSG_PLAYERQUIT;
	msg.genEvent.controller = n;

	if (!Server->player[n].isConnected) {
          SDLNet_TCP_Close(Server->player[n].socket);
          SDLNet_TCP_DelSocket(Server->socketSet, Server->player[n].socket);
          Server->player[n].isConnected = FALSE;
          msBroadcastMessage(Server, &msg);
        }
}

void msAcceptConnection(MupenServer *Server) {
  TCPsocket newClient;
  NetMessage msg;

  int n, allPlayersReady = 1;

  SDLNet_CheckSockets(Server->socketSet, 0);
  if (SDLNet_SocketReady(Server->socket)) {
    if (newClient = SDLNet_TCP_Accept(Server->socket)) {
        for (n = 0; n < MAX_CLIENTS; n++)
          if (!Server->player[n].isConnected) {
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
    msStopWaitingForPlayers(Server);
    msg.type = NETMSG_SYNC;
    msg.genEvent.timer = getEventCounter();
    msBroadcastMessage(Server, &msg);    
  } 
 
}

void msProcessMessages(MupenServer *Server) {
  NetMessage incomingMsg;
  int tempReturn, n;

  SDLNet_CheckSockets(Server->socketSet, 0);

  for (n = 0; n < MAX_CLIENTS; n++) {
    if ((Server->player[n].isConnected) && (SDLNet_SocketReady(Server->player[n].socket))) {
      tempReturn = SDLNet_TCP_Recv(Server->player[n].socket, &incomingMsg, sizeof(NetMessage));

      if (tempReturn <= 0) {                                     
        msBootPlayer(Server, n);                                     // If there was an error or the player disconnected then cleanup.
      }
      else {                   
        switch (incomingMsg.type) {
            case NETMSG_EVENT:                                       // Events (Button presses, other time sensitive input)
              if (incomingMsg.genEvent.type == EVENT_BUTTON) {
                    incomingMsg.genEvent.timer = getEventCounter() + Server->netDelay;      // Add calculated delay to the timer
                    if (n < 4) {                                     // If the message comes from a player (n >= 4 is for spectators)
                        incomingMsg.genEvent.controller = n;         // Change the controller tag on the event
                        msBroadcastMessage(Server, &incomingMsg);    // Broadcast the button event
                    }
              }
              break;
            case NETMSG_READY: // Client is notifying server that it is ready to begin
                Server->player[n].isReady = TRUE;
                break;
            case NETMSG_PING: // Client is returning a ping request
                Server->player[n].lag = (gettimeofday_msec() - incomingMsg.genEvent.value);
                break;
            case NETMSG_DESYNC: // Client is notifying the server that it has desynchronized
                incomingMsg.genEvent.controller = n;
                msBroadcastMessage(Server, &incomingMsg);
                break;
        }
      }
    }
  }
}

