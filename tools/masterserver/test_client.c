/*

  A bunch of copy & paste crap thrown together to test 
  the master server with...

  I don't take any responsibility for it.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL_net.h>
#include <time.h>
#include <arpa/inet.h>

#define FIND_GAMES      00
#define GAME_LIST	01
#define OPEN_GAME       02
#define GAME_DESC	03
#define KEEP_ALIVE      04 

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    #define GET_IP(ip)      ((unsigned char*)&(ip))[3],((unsigned char*)&(ip))[2], \
                            ((unsigned char*)&(ip))[1],((unsigned char*)&(ip))[0]
#else
    #define GET_IP(ip)      ((unsigned char*)&(ip))[0],((unsigned char*)&(ip))[1], \
                            ((unsigned char*)&(ip))[2],((unsigned char*)&(ip))[3]
#endif

typedef unsigned char md5_byte_t;

void random_md5(md5_byte_t *md5) {
  int n;
  for (n = 0; n < 16; n++) {
     md5[n] = (rand() % 255);
  }
}
void print_md5(md5_byte_t md5[16]) {
  int n;
  for (n = 0; n < 16; n++) {
     printf("%x", md5[n]);
  }
}
  
int main(int argc, char **argv)
{
        md5_byte_t md5[16];
        uint16_t temp_port;
        int n;

	UDPsocket sd;
	IPaddress srvadd;
	UDPpacket *p;
        int button, gameDesc = 0;
 
        srand(time(NULL));
        random_md5((md5_byte_t *)&md5);

	/* Check for parameters */
	if (argc < 3)
	{
		fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
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

        while (1) {
            p->data[0] = 'M';
            p->data[1] = '+';
            p->data[2] = 1;
            p->data[3] = 0; 

            printf("## M64+ Master Server Test Client\n");
            printf("## Currently connected to %d.%d.%d.%d:%d\n", GET_IP(p->address.host), ntohs(p->address.port));
            printf("## MD5: ");
            print_md5(md5);
            printf("\n");
            printf("## Game Desc: %d\n\n", gameDesc);
            printf("1. Send OPEN_GAME packet.\n");
            printf("2. Send KEEP_ALIVE packet.\n");
            printf("3. Send FIND_GAMES packet.\n");
            printf("4. Randomize MD5.\n");
            printf("> ");
            while ((button = getchar()) == 10) {}
            button -= 48;
            printf("\n");
            system("clear");
            switch(button) {
                  case 1:
                     p->data[4] = OPEN_GAME;
                     memcpy(p->data+5, &md5, 16);
                     SDLNet_Write16(666, p->data + 21);
                     p->len = 23;
                     SDLNet_UDP_Send(sd, -1, p);
                     while (SDLNet_UDP_Recv(sd, p) == 0) {}
                     gameDesc = SDLNet_Read16(p->data + 1);
                     printf("Game descriptor %d returned.\n\n", gameDesc);
                     break;
                  case 2:
                     p->data[4] = KEEP_ALIVE;
                     SDLNet_Write16(gameDesc, p->data + 5);
                     p->len = 7;
                     SDLNet_UDP_Send(sd, -1, p);
                     break;
                  case 3:
                     p->data[4] = FIND_GAMES;
                     memcpy(p->data+5, &md5, 16);
                     p->len = 21;
                     SDLNet_UDP_Send(sd, -1, p);

                     do {
                       while (SDLNet_UDP_Recv(sd, p) == 0) {}
                       for (n = 0; n < (p->len - 2) / 6; n++) {
                         temp_port = SDLNet_Read16(p->data + (n*6+6));
                         printf("%d.%d.%d.%d:%d\n", GET_IP( *(p->data + (n*6+2)) ), temp_port);
                       }
                     } while (p->data[1]);
                     break;

                  case 4:
                     random_md5((md5_byte_t *)&md5);
                     break;
                  case 5:
                     break;
            }
        }

	SDLNet_FreePacket(p);
	SDLNet_Quit();
 
	return EXIT_SUCCESS;
}
