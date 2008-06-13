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

MupenServer		Server;

BOOL            serverIsActive() {return Server.isActive;}
BOOL            serverIsAccepting() {return Server.isAccepting;}
unsigned short  getNetDelay() {return Server.netDelay;}


void serverInitialize() {
        memset(&Server, 0, sizeof(Server));
}

int serverStart(unsigned short port) {
        IPaddress serverAddr;

	fprintf((FILE *)getNetLog(), "serverStart() called.\n");
	if (serverIsActive()) return;

	Server.socketSet = SDLNet_AllocSocketSet(MAX_CLIENTS + 1);
	SDLNet_ResolveHost(&serverAddr, NULL, port);
	
	if (Server.socket = SDLNet_TCP_Open(&serverAddr)) {
		SDLNet_TCP_AddSocket(Server.socketSet, Server.socket);
		fprintf((FILE *)getNetLog(), "Server successfully initialized on port %d.\n", port);
                Server.isActive = 1;
		Server.isAccepting = 1;
	}
	else fprintf((FILE *)getNetLog(), "Failed to initialize server on port %d.\n", port);
	return Server.isActive;
}

void serverStop() {
  int n;
  fprintf((FILE *)getNetLog(), "serverStop() called.\n");
  for (n = 0; n < MAX_CLIENTS; n++) if (Server.player[n].isConnected) serverBootPlayer(n);
  SDLNet_TCP_Close(Server.socket);
  SDLNet_FreeSocketSet(Server.socketSet);
  Server.isActive = 0;
  Server.isAccepting = 0;
}

void serverStopWaitingForPlayers() {
	SDLNet_TCP_Close(Server.socket);
	SDLNet_TCP_DelSocket(Server.socketSet, Server.socket);
        Server.isAccepting = 0;
}

int serverBroadcastMessage(NetMessage *msg) {
  int n;
  for (n = 0; n < MAX_CLIENTS; n++) if (Server.player[n].isConnected) {
	if (SDLNet_TCP_Send(Server.player[n].socket, msg, sizeof(NetMessage)) != sizeof(NetMessage)) {
		serverBootPlayer(n);                   // If the player disconnected, clean up.
	}
  }
}

void serverBootPlayer(int n) {
	NetMessage msg;
	msg.type = NETMSG_PLAYERQUIT;
	msg.genEvent.controller = n;

	SDLNet_TCP_Close(Server.player[n].socket);
	SDLNet_TCP_DelSocket(Server.socketSet, Server.player[n].socket);
        Server.player[n].isConnected = FALSE;
	fprintf((FILE *)getNetLog(), "Client %d disconnected.\n", n);
	serverBroadcastMessage(&msg);
}

void serverAcceptConnection() {
  TCPsocket newClient;
  NetMessage msg;

  int n;
  SDLNet_CheckSockets(Server.socketSet, 0);
  if (SDLNet_SocketReady(Server.socket)) {
    if (newClient = SDLNet_TCP_Accept(Server.socket)) {
        for (n = 0; n < MAX_CLIENTS; n++)
          if (!Server.player[n].isConnected) {
            SDLNet_TCP_AddSocket(Server.socketSet, (Server.player[n].socket = newClient));
            fprintf((FILE *)getNetLog(), "New connection accepted; Dubbed Player %d\n", n + 1);
            Server.player[n].isConnected = TRUE;
            fprintf((FILE *)getNetLog(), "Sending ping.\n");
            msg.type = NETMSG_PING;
            msg.genEvent.type = NETMSG_PING;
            msg.genEvent.value = gettimeofday_msec();
            SDLNet_TCP_Send(newClient, &msg, sizeof(NetMessage));
            break;
          }
        if (n == MAX_CLIENTS) SDLNet_TCP_Close(newClient); // No open slots
    }
  }
}

void serverProcessMessages() {
  NetMessage incomingMsg;
  int tempReturn, n;

  SDLNet_CheckSockets(Server.socketSet, 0);

  for (n = 0; n < MAX_CLIENTS; n++) {
    if ((Server.player[n].isConnected) && (SDLNet_SocketReady(Server.player[n].socket))) {
      tempReturn = SDLNet_TCP_Recv(Server.player[n].socket, &incomingMsg, sizeof(NetMessage));

      if (tempReturn <= 0) {                                     
        serverBootPlayer(n);                                     // If there was an error or the player disconnected then cleanup.
      }
      else {                   
        switch (incomingMsg.type) {
            case NETMSG_EVENT:                                   // Events (Button presses, other time sensitive input)
              if (incomingMsg.genEvent.type == EVENT_BUTTON) {
                    incomingMsg.genEvent.timer += Server.netDelay;      // Add calculated delay to the timer
                    if (n < 4) {                                 // If the message comes from a player (n >= 4 is for spectators)
                        incomingMsg.genEvent.controller = n;     // Change the controller tag on the event
                        serverBroadcastMessage(&incomingMsg);    // Broadcast the button event
                    }
              } else {
                    fprintf((FILE *)getNetLog(), "Server: Received unrecognized event from player %d.\n", n+1);
              }
              break;
            case NETMSG_PING:
                Server.player[n].lag = (gettimeofday_msec() - incomingMsg.genEvent.value);
                fprintf((FILE *)getNetLog(), "Server: Ping returned from player %d, lag %d ms.\n", n+1, Server.player[n].lag);
                break;
            case NETMSG_DESYNC:
                incomingMsg.genEvent.controller = n;
                serverBroadcastMessage(&incomingMsg);
                //serverBootPlayer(n); When testing is finished
                break;
        }
      }
    }
  }
}

void serverBroadcastSync() {
  int tc = 0, ptr = 0, n, lc;
  NetMessage startmsg;
  struct timespec ts;
  static short sort_array[MAX_CLIENTS];
  static BOOL runOnce = FALSE;


  if (!runOnce) {
    runOnce = TRUE;
    serverStopWaitingForPlayers();
    fprintf((FILE *)getNetLog(), "Server: F9 pressed\n");
    for (n = 0; n < MAX_CLIENTS; n++) sort_array[n] = -1;

    // Sort out connection lag from highest to lowest
    for (n = 0; n < MAX_CLIENTS; n++)
        if  ( (Server.player[n].lag >= Server.player[tc].lag)
           && (Server.player[n].isConnected) ) 
              tc = n;

    sort_array[ptr] = tc;
    lc = tc;
    tc = 0;

    for (ptr = 1; ptr < MAX_CLIENTS; ptr++) {
       for (n = 0; n < MAX_CLIENTS; n++) 
            if ( (Server.player[n].lag >= Server.player[tc].lag)
              && (Server.player[n].lag <= Server.player[lc].lag) 
              && (n != lc) 
              && (Server.player[n].isConnected))
                 tc = n;
       if (tc == lc) break;
       sort_array[ptr] = tc;
       lc = tc;
       tc = 0;
    }

    fprintf((FILE *)getNetLog(), "Client Lag(ms):\n");
    for (n = 0; n < MAX_CLIENTS; n++) 
          if (Server.player[n].isConnected) fprintf((FILE *)getNetLog(),"   Player %d: %d ms\n", sort_array[n]+1, Server.player[sort_array[n]].lag);

    Server.netDelay = (Server.player[sort_array[0]].lag / 17); // 60 VI/s = ~17ms per VI (+ 7 to be safe)
    fprintf((FILE *)getNetLog(), "Net Delay: %d\n", Server.netDelay);
  }

  startmsg.type = NETMSG_SYNC;
  startmsg.genEvent.timer = getEventCounter();

  // Send sync signals, in order of slowest to fastest connection, compensate for net lag
  for (n = 0; n < MAX_CLIENTS; n++) {
     ptr = sort_array[n];
     lc  = sort_array[n+1];
     if (ptr >= 0) {
       fprintf((FILE *)getNetLog(), "Sending sync message to Player %d (ping %d ms)\n", ptr + 1, Server.player[ptr].lag);
       SDLNet_TCP_Send(Server.player[ptr].socket, &startmsg, sizeof(NetMessage));
       if (lc >= 0) {
         ts.tv_sec = 0;
         ts.tv_nsec = 1000000 * (Server.player[ptr].lag - Server.player[lc].lag) / 2; // Divide by 2 for one way trip
         fprintf((FILE *)getNetLog(), "Sleeping for %d ms\n", (Server.player[ptr].lag - Server.player[lc].lag) / 2);
         nanosleep(&ts, NULL);
       }
     }
  }
  fprintf((FILE *)getNetLog(), "Finished sending sync messages.\n");
}
