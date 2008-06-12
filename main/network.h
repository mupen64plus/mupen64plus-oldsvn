#define SERVER_PORT		7000
#define MAX_CLIENTS		10


#define 	NETMSG_EVENT		0
#define		NETMSG_STARTEMU		1	// Begin execution of ROM code
#define		NETMSG_PING		2
#define		NETMSG_SYNC		3

#define 	NET_INT_FREQ		3000 // This should be down to the milli second

// Client to Server
#define 	NETMSG_BUTTON		0
#define		NETMSG_REQUEST		100	// Request to join a game

// Server to Client

#define		NETMSG_ALLOW		202	// Accept request to join a game
#define		NETMSG_DENY		203	// Reject request to join a game

#define		NETMSG_NEWPLAYER	204	// New player announcement
#define		NETMSG_PLAYERQUIT	205	// Player disconnect
#define		NETMSG_SETNAME		206


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

void netInteruptLoop();

u_int16_t getSyncCounter();
void setSyncCounter(u_int16_t v);
unsigned short netIsConnected();
DWORD getNetKeys(int control);
void setNetKeys(int control, DWORD value);
unsigned short	clientIsConnected();
unsigned short	netServerIsActive();
unsigned short  serverWaitingForPlayers();

void netInitialize();
void netShutdown();

void netSendButtonState(int control, DWORD value);
int clientRecvMessage(NetMessage *msg);
int clientSendMessage(NetMessage *msg);
void clientProcessMessages();

int clientConnect(char *server, int port);
void clientDisconnect();
void netProcessMessages();

int serverStart(unsigned short port);
void serverStop();
void serverStopListening(); // Call this when emulator starts
void serverProcessMessages();
void serverAcceptConnection();
void serverKillClient(int n);

void netAddEvent(unsigned short type, int controller, DWORD value, unsigned short timer);
int netGetNextEvent(unsigned short *type, int *controller, DWORD *value, unsigned short *timer);
void netPopEvent();
void netProcessEventQueue();
void netInitEventQueue();
void netKillEventQueue() ;

void netMain();
