/* =======================================================================================

	network.h
	by orbitaldecay

   =======================================================================================
*/ 
#include "../main/main.h"
#include "../r4300/r4300.h"
#include "../opengl/osd.h"
#include "../main/plugin.h"
#include <sys/types.h>
#include <SDL_net.h>

#define SERVER_PORT		7000
#define MAX_CLIENTS		10

#define 	NETMSG_EVENT		0       // Time sensitive input events
#define		NETMSG_STARTEMU		1	// Begin execution of ROM code
#define		NETMSG_PING		2       // Used to detect latency
#define		NETMSG_DESYNC		3       // Client sends this when they've desynced
#define		NETMSG_PLAYERQUIT	4	// Player disconnect

#define 	EVENT_BUTTON		0


// I made sure to use integer types here that were safe to send between 32bit and 64bit platforms
typedef struct TNetEvent {
        u_int8_t		type;
	u_int8_t		controller;	// applicable controller
	u_int32_t		value;		// new key state value to assign (BUTTONS.Value)
	u_int16_t		timer;		// when to activate the event (== netVISyncCounter)
	struct TNetEvent	*next;		// Next button event in queue
} NetEvent;

typedef struct TNetMessage {
	u_int8_t	id;
	u_int16_t	type;
        struct {
            u_int8_t		type;
            u_int8_t		controller;	// applicable controller
            u_int32_t		value;		// new key state value to assign (BUTTONS.Value)
            u_int16_t		timer;		// when to activate the event (== netVISyncCounter)
        } genEvent;
} NetMessage;

typedef struct TNetPlayer {
        short      lag;
        TCPsocket  socket;
        char       nick[20];
        BOOL       isConnected;
} NetPlayer;

typedef struct TMupenClient {
        u_int16_t          eventCounter;            // Track VI in order to syncrhonize button events over network
        NetEvent          *eventQueue;              // Pointer to queue of upcoming button events
        TCPsocket          socket;                  // Socket descriptor for connection to server
        SDLNet_SocketSet   socketSet;               // Set for client connection to server
        BUTTONS            playerKeys[MAX_CLIENTS];
        BOOL               isConnected;
        BOOL               isWaitingForServer;
} MupenClient;

typedef struct TMupenServer {
	TCPsocket        socket;        // Socket descriptor for server
	SDLNet_SocketSet socketSet;     // Set of all connected clients, along with the server socket descriptor
	int		 netDelay;      // For LCD network latency
        NetPlayer        player[MAX_CLIENTS];
        BOOL             isActive;
        BOOL		 isAccepting;
} MupenServer;

typedef struct TNetPlaySettings {
        BOOL             runServer;
        char             hostname[128];
        u_int16_t        port;
} NetPlaySettings;

void net_init();
void netEmuStart();
void netShutdown();
void netInteruptLoop();

DWORD		getNetKeys(int control);
void		setNetKeys(int control, DWORD value);
u_int16_t	getEventCounter();

void serverInitialize();
BOOL serverIsActive();
int serverStart(unsigned short port);
void serverStop();
void serverStopListening();
void serverProcessMessages();
void serverAcceptConnection();
void serverBootPlayer(int n);

void clientInitialize();
int clientRecvMessage(NetMessage *msg);
int clientSendMessage(NetMessage *msg);
void clientProcessMessages();
int clientConnect(char *server, int port);
void clientDisconnect();
void netProcessMessages();
void addEventToQueue(unsigned short type, int controller, DWORD value, unsigned short timer);
int netGetNextEvent(unsigned short *type, int *controller, DWORD *value, unsigned short *timer);
BOOL clientIsConnected();
BOOL clientWaitingForServer();
void popEventQueue();
void processEventQueue();
void initEventQueue();
void killEventQueue();
void clientSendButtons(int control, DWORD value);



