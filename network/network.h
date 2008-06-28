/* =======================================================================================

	network.h
	by orbitaldecay

   =======================================================================================
*/ 

#include "../opengl/osd.h"
#include "../main/plugin.h" // Need typdef BUTTONS, DWORD, BOOL
#include "../main/main.h"
#include <sys/types.h>
#include <SDL_net.h>

#ifdef _WIN32               //needed for gethostname in linux, in windows it's Winsock2.h
    #include "Winsock2.h"
#else
    #include "unistd.h"
#endif

#define SERVER_PORT		7000

#define MAX_PLAYERS         4
#define MAX_CLIENTS         10
#define MAX_PACKET_SIZE     1024
#define VI_PER_FRAME        4
#define FRAME_BUFFER_LENGTH 64  // should be nearly double max frame latency
#define QUEUE_HEAP_LEN      64  // number of items in priority queue

#define QUEUE_JOIN_DELAY    120 // delay between connect attempts in frames

#define FRAME_MASK          0x7FFF
#define	FRAME_JOINREQUEST	0xFFFF  // Sent when asking to join the network
#define FRAME_JOIN      	0xFFFE  // Response sent to new peer (contains other clients & new node's id)

#define 	NETMSG_JOIN		    0   // Check state of join in progress
#define		NETMSG_PAUSE		1	// Continue/pause execution of ROM code
#define		NETMSG_PING		    2       // Used to detect latency
#define		NETMSG_DESYNC		3       // Client sends this when they've desynced
#define		NETMSG_PLAYERQUIT	4	// Player disconnect
#define     NETMSG_WAIT         6       // Client telling server to slow down
#define		NETMSG_READY		5       // Tell server that client is ready
#define		NETMSG_JOINNOTIFY	6	// Sent to establish a new peer's info on network
#define		NETMSG_JOINREPLY	7	// Replies for establishing new peer's info

#define 	EVENT_BUTTON		0

// I made sure to use integer types here that were safe to send between 32bit and 64bit platforms

typedef struct TFrame {
    Uint16  eID;
    Uint8   peer;
    Sint8   lag;
} Frame;//sizeof=4

typedef struct TJoinRequest {
    Uint16      eID;
    IPaddress   client;
} JoinRequest;//sizeof=8

typedef struct TJoinReply {
    
} JoinReply;//sizeof=

typedef struct TJoinState {
    enum States {
        enabled = 0,    //0 is default state
        disabled,
        servicing,
        waiting
    }               state;
    JoinRequest     packet;
    IPaddress       host;
} JoinState;

typedef struct TNetTask {
    unsigned int    time;
    u_int8_t	    msg;
} NetTask;

typedef struct TNetEvent {
    unsigned int    time;
    NetTask         task;
} NetEvent;

typedef struct TNetPlayerUpdate {
    u_int32_t       value;
    u_int16_t       timer;
    u_int16_t       reserved;   //Make this a round 64 bits for 32-bit alignment
} NetPlayerUpdate;

typedef struct TNetPlayer {
        short      lag;
        //TCPSocket  socket;
        IPaddress  address;
        char       nick[20];
        BOOL       isConnected;
        BOOL       isReady;
        //BUTTONS     keys;
} NetPlayer;

typedef struct TMupenClient {
        unsigned int    frameCounter;               // Tracks current frame, synchronized counter

        NetEvent        *eventQueue[QUEUE_HEAP_LEN];//Pointers to queue of upcoming events
        NetEvent        events[QUEUE_HEAP_LEN];
        unsigned int    numQueued;

        JoinState       joinState;

        UDPsocket          socket;                  //listening socket
        SDLNet_SocketSet   socketSet;               // Set for client connection to ms
        UDPpacket          *packet;
        BUTTONS            playerKeys[MAX_CLIENTS];
        NetPlayer           player[MAX_CLIENTS];
        NetPlayerUpdate     playerevent[FRAME_BUFFER_LENGTH][MAX_PLAYERS];
        u_int16_t           oldestWaiting[MAX_PLAYERS];
        u_int16_t          lastSync;
        char	            lag[MAX_PLAYERS];
        char                lag_local[MAX_PLAYERS];
        u_int8_t           syncFreq;
        u_int32_t               numConnected;
        u_int32_t               myID;
        BOOL               isWaitingForServer;
        BOOL                isListening;
} MupenClient;

typedef struct TNetPlaySettings {
        //BOOL             runServer;
        char             hostname[128];
} NetPlaySettings;

//Note that the IP should always be in network byte order
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    #define GET_IP(ip)      ((unsigned char*)&(ip))[3],((unsigned char*)&(ip))[2], \
                            ((unsigned char*)&(ip))[1],((unsigned char*)&(ip))[0]
#else
    #define GET_IP(ip)      ((unsigned char*)&(ip))[0],((unsigned char*)&(ip))[1], \
                            ((unsigned char*)&(ip))[2],((unsigned char*)&(ip))[3]
#endif
#define GET_PORT(port) SDLNet_Read16(&port)

int netInitialize(MupenClient *mClient);
int netStartNetplay(MupenClient *mClient, NetPlaySettings netSettings);
void netShutdown(MupenClient *mClient);
int netMain(MupenClient *mClient);

int clientInitialize(MupenClient *mClient);
int clientSendMessage(MupenClient *Client);
void clientProcessMessages(MupenClient *Client);
int clientConnect(MupenClient *Client, char *server, int port);
void clientDisconnect(MupenClient *Client);
void clientSendButtons(MupenClient *Client, int control, DWORD value);

void heapifyEventQueue(MupenClient *Client,unsigned int elem);
void addEventToQueue(MupenClient *Client);
void popEventQueue(MupenClient *Client);
void processEventQueue(MupenClient *Client);
void flushEventQueue(MupenClient *Client);

int frameDelta(MupenClient *Client, Uint32 frame);
int sourceID(int myID, int index);

