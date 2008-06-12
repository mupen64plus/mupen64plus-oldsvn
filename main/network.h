/* =======================================================================================

	network.h
	by orbitaldecay

	KNOWN BUGS THAT NEED FIXING:

	When playing multiplayer, all of the controllers must be enabled in the plugin
	if input is being received over the net.  Haven't found an easy way of fixing
	this yet.

   =======================================================================================
*/ 
#include <SDL_net.h>

#define SERVER_PORT		7000
#define MAX_CLIENTS		10

#define 	NETMSG_EVENT		0
#define		NETMSG_STARTEMU		1	// Begin execution of ROM code
#define		NETMSG_PING		2
#define		NETMSG_SYNC		3

#define 	NETMSG_BUTTON		0
#define		NETMSG_PLAYERQUIT	205	// Player disconnect



typedef struct TNetPlayer {
        BUTTONS    keys;
        short      lag;
        TCPsocket  socket;
        char       nick[20];
        BOOL       isConnected;
} NetPlayer;

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

void netInitialize();
void netShutdown();
void netInteruptLoop();
void clientSendButtons(int control, DWORD value);

DWORD		getNetKeys(int control);
u_int16_t	getSyncCounter();
void		setNetKeys(int control, DWORD value);
void		setSyncCounter(u_int16_t v);
unsigned short	clientIsConnected();
unsigned short	netServerIsActive();
unsigned short  serverWaitingForPlayers();


