/*

  A bunch of copy & paste crap thrown together to test 
  the master server with...

  I don't take any responsibility for it.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL_net.h>

#define FIND_GAMES      00
#define GAME_LIST	01
#define OPEN_GAME       02
#define GAME_DESC	03
#define KEEP_ALIVE      04 

int main(int argc, char **argv)
{
	UDPsocket sd;
	IPaddress srvadd;
	UDPpacket *p;
 
	/* Check for parameters */
	if (argc < 4)
	{
		fprintf(stderr, "Usage: %s <host> <port> <OPEN_GAME/FIND_GAMES/KEEP_ALIVE>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	/* Initialize SDL_net */
	if (SDLNet_Init() < 0)
	{
		fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}
	
	/* Open a socket on random port */
	if (!(sd = SDLNet_UDP_Open(0)))
	{
		fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}
 
	/* Resolve server name  */
	if (SDLNet_ResolveHost(&srvadd, argv[1], atoi(argv[2])))
	{
		fprintf(stderr, "SDLNet_ResolveHost(%s %d): %s\n", argv[1], atoi(argv[2]), SDLNet_GetError());
		exit(EXIT_FAILURE);
	}
 
	/* Allocate memory for the packet */
	if (!(p = SDLNet_AllocPacket(512)))
	{
		fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}
 
        p->address.host = srvadd.host; /* Set the destination host */
        p->address.port = srvadd.port; /* And destination port */

        if (strcmp(argv[3], "OPEN_GAME") == 0) {
            p->data[0] = OPEN_GAME;
            SDLNet_Write16(666, p->data + 17);
            p->len = 19;
            SDLNet_UDP_Send(sd, -1, p);

            while (SDLNet_UDP_Recv(sd, p) == 0) {}
            printf("Packet Type: %d\n", (char)p->data[0]);
            printf("Value:       %d\n", SDLNet_Read16(p->data + 1));
        }

        if (strcmp(argv[3], "KEEP_ALIVE") == 0) {
            p->data[0] = KEEP_ALIVE;
            SDLNet_Write16(0, p->data + 1);
            p->len = 3;
            SDLNet_UDP_Send(sd, -1, p);
        }

	SDLNet_FreePacket(p);
	SDLNet_Quit();
 
	return EXIT_SUCCESS;
}
