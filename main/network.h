#define netDelay		16

#define SERVER_PORT		7000
#define MAX_CLIENTS		10

#define 	NETMSG_BUTTON		0
#define		NETMSG_STARTEMU		201	// Begin execution of ROM code

#define 	NETMSG_EVENT		0

#define 	NET_INT_FREQ		3000 // This should be down to the milli second

// Client to Server
#define		NETMSG_REQUEST		100	// Request to join a game

// Server to Client

#define		NETMSG_ALLOW		202	// Accept request to join a game
#define		NETMSG_DENY		203	// Reject request to join a game

#define		NETMSG_NEWPLAYER	204	// New player announcement
#define		NETMSG_PLAYERQUIT	205	// Player disconnect
#define		NETMSG_SETNAME		206

typedef struct TNetEvent {
        unsigned char		type;
	unsigned char		controller;	// applicable controller
	DWORD			value;		// new key state value to assign (BUTTONS.Value)
	unsigned short		timer;		// when to activate the event (== netVISyncCounter)
	struct TNetEvent	*next;		// Next button event in queue
} NetEvent;

typedef struct TNetMessage {
	unsigned short	id;
	unsigned char	type;
	NetEvent genEvent;
} NetMessage;

typedef struct TNetJoinRequest {
	unsigned char		controller;
	char			nickname[12];
} NetJoinRequest;

typedef struct TNetServerInfo {
	unsigned char		availableSlots;
	char			gameMD5;
} NetServerInfo;

void netInteruptLoop();

unsigned short getSyncCounter();
void setSyncCounter(unsigned short v);
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
