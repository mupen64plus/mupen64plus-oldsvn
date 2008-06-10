#define netDelay		7  // More than enough to compensate for latency in connections with sub 100ms ping

#define SERVER_PORT		7000
#define MAX_CLIENTS		10

#define 	NETMSG_BUTTON		0

// Client to Server
#define		NETMSG_REQUEST		100	// Request to join a game

// Server to Client
#define		NETMSG_STARTEMU		201	// Begin execution of ROM code
#define		NETMSG_ALLOW		202	// Accept request to join a game
#define		NETMSG_DENY		203	// Reject request to join a game

#define		NETMSG_NEWPLAYER	204	// New player announcement
#define		NETMSG_PLAYERQUIT	205	// Player disconnect
#define		NETMSG_SETNAME		206

typedef struct TNetButtonEvent {
	unsigned char		controller;	// applicable controller
	DWORD			value;		// new key state value to assign (BUTTONS.Value)
	unsigned short		timer;		// when to activate the event (== netVISyncCounter)
	struct TNetButtonEvent	*next;		// Next button event in queue
} NetButtonEvent;

typedef struct TNetJoinRequest {
	unsigned char		controller;
	char			nickname[12];
} NetJoinRequest;

typedef struct TNetServerInfo {
	unsigned char		availableSlots;
	char			gameMD5;
} NetServerInfo;

typedef struct TNetMessage {
	unsigned short	id;
	unsigned char	type;
	union {
		char rawData[32];
		NetButtonEvent buttonEvent;
		NetJoinRequest joinRequest;
		NetServerInfo  serverInfo;
	} data;
} NetMessage;

unsigned short netGetSyncCounter();
void netSetSyncCounter(unsigned short v);
unsigned short netIsConnected();
DWORD netGetKeys(int control);
void netSetKeys(int control, DWORD value);
unsigned short	netClientIsConnected();
unsigned short	netServerIsActive();

void netInitialize();
void netSendButtonState(int control, DWORD value);
int netClientRecvMessage(NetMessage *msg);
int netClientSendMessage(NetMessage *msg);
void netClientProcessMessages();

int clientConnect(char *server, int port);
void clientDisconnect();
void netProcessMessages();

int serverStart(unsigned short port);
void serverStop();
void serverProcessMessages();
void serverAcceptConnection();
void serverKillClient(int n);

void netAddButtonEvent(int controller, DWORD value, unsigned short timer);
int netGetNextButtonEvent(int *controller, DWORD *value, unsigned short *timer);
void netPopButtonEvent();
void netProcessButtonQueue();

void netMain();
