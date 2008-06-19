/* =======================================================================================

	network.h
	by orbitaldecay

   =======================================================================================
*/ 

#include "../opengl/osd.h"
#include "../main/plugin.h" // Need typdef BUTTONS, DWORD, BOOL
#include <sys/types.h>
#include <SDL_net.h>

#define SERVER_PORT		7000


#define MAX_CLIENTS		10


#define 	NETMSG_EVENT		0       // Time sensitive input events
#define		NETMSG_SYNC		1	// Continue/begin execution of ROM code
#define		NETMSG_PING		2       // Used to detect latency
#define		NETMSG_DESYNC		3       // Client sends this when they've desynced
#define		NETMSG_PLAYERQUIT	4	// Player disconnect
#define         NETMSG_WAIT             6       // Client telling server to slow down
#define		NETMSG_READY		5       // Tell server that client is ready

#define 	EVENT_BUTTON		0

#define         SYNC_PERFECT            0
#define         SYNC_AHEAD              1
#define         SYNC_BEHIND             2


// I made sure to use integer types here that were safe to send between 32bit and 64bit platforms
typedef struct TNetEvent {
        u_int8_t		type;
	u_int8_t		controller;	// applicable controller
	u_int32_t		value;		// new key state value to assign (BUTTONS.Value)
	u_int16_t		timer;		// when to activate the event (== netVISyncCounter)
        u_int32_t               time_stamp;
	struct TNetEvent	*next;		// Next button event in queue
} NetEvent;

typedef struct TNetMessage {
        struct {
            u_int32_t		value;		// new key state value to assign (BUTTONS.Value)
            u_int16_t		timer;		// when to activate the event (== netVISyncCounter)
            u_int8_t		type;
            u_int8_t		player;		// applicable controller
        } genEvent;
        u_int32_t       time_stamp;
	u_int16_t	type;
} NetMessage;

typedef struct TNetPlayer {
        short      lag;
        TCPsocket  socket;
        char       nick[20];
        BOOL       isConnected;
        BOOL       isReady;
} NetPlayer;

typedef struct TMupenClient {
        u_int16_t          frameCounter;            // Track frames in order to syncrhonize button events over network
        NetEvent          *eventQueue;              // Pointer to queue of upcoming button events
        TCPsocket          socket;                  // Socket descriptor for connection to ms
        SDLNet_SocketSet   socketSet;               // Set for client connection to ms
        BUTTONS            playerKeys[MAX_CLIENTS];
        u_int16_t          lastSync;
        u_int16_t	   lag;
        u_int8_t           syncFreq;
        BOOL               isConnected;
        BOOL               isWaitingForServer;
} MupenClient;

typedef struct TMupenServer {
	TCPsocket        socket;        // Socket descriptor for ms
	SDLNet_SocketSet socketSet;     // Set of all connected clients, along with the ms socket descriptor
	int		 netDelay;      // For LCD network latency
        NetPlayer        player[MAX_CLIENTS];
        BOOL             isActive;
        BOOL		 isAccepting;
} MupenServer;

typedef struct TNetPlaySettings {
        BOOL             runServer;
        char             hostname[128];
} NetPlaySettings;

int netStartNetplay(MupenServer *mServer, MupenClient *mClient, NetPlaySettings netSettings);
void netShutdown(MupenServer *mServer, MupenClient *mClient);
int netMain(MupenServer *mServer, MupenClient *mClient);

void serverInit(MupenServer *Server);
int serverStart(MupenServer *Server, unsigned short port);
void serverStop(MupenServer *Server);
void serverAccept(MupenServer *Server);
void serverStopWaitingForPlayers(MupenServer *Server);
void serverProcessMessages(MupenServer *Server);
void serverBootPlayer(MupenServer *Server, int n);
int serverBroadcastMessage(MupenServer *Server, NetMessage *msg);

int clientSendMessage(MupenClient *Client, NetMessage *msg);
void clientProcessMessages(MupenClient *Client);
int clientConnect(MupenClient *Client, char *server, int port);
void clientDisconnect(MupenClient *Client);
void clientSendButtons(MupenClient *Client, int control, DWORD value);

void addEventToQueue(MupenClient *Client, NetMessage msg);
void popEventQueue(MupenClient *Client);
void processEventQueue(MupenClient *Client);
void flushEventQueue(MupenClient *Client);



