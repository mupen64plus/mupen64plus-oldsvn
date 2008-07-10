/**
 * Mupen64Plus - client.c
 * 2008 orbitaldecay & DarkJeztr
 *
 * Mupen64Plus homepage: http://code.google.com/p/mupen64plus/
 * 
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
 * You should have received a copy of the GNU General Public
 * Licence along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
**/

#include <time.h>
#include "network.h"
#include "../main/md5.h"
#include "../r4300/r4300.h"

int clientInitialize(MupenClient *Client) {
    int i;
    memset(Client, 0, sizeof(Client));

    if (!(Client->socketSet = SDLNet_AllocSocketSet(MAX_CLIENTS))) {
        printf("[Netplay] clientini: failure to allocate socket set.\n");

    } else if (!(Client->socket = SDLNet_UDP_Open(SERVER_PORT))) {
        printf("[Netplay] clientini: failure to open UDP port %d\n", SERVER_PORT);
        SDLNet_FreeSocketSet(Client->socketSet);

    } else if (!(Client->packet = SDLNet_AllocPacket(MAX_PACKET_SIZE))) {
        printf("[Netplay] clientini: failure to allocate packet buffer.\n");
        SDLNet_FreeSocketSet(Client->socketSet);
        SDLNet_UDP_Close(Client->socket);

    } else if (SDLNet_UDP_AddSocket(Client->socketSet, Client->socket) <= 0) {
        printf("[Netplay] clientini: failure to add client socket to socket set.\n");
        SDLNet_FreeSocketSet(Client->socketSet);
        SDLNet_UDP_Close(Client->socket);
        SDLNet_FreePacket(Client->packet);

    } else {
        Client->isListening = 1;
        Client->inputDelay=DEFAULT_INPUT_DELAY;
        for(i=0; i<QUEUE_HEAP_LEN; i++)
            Client->eventQueue[i]=&(Client->events[i]);
        printf("[Netplay] Client successfully initialized.\n");
    }

    return Client->isListening;
}

int clientConnect(MupenClient *Client, char *server, int port) {
	//IPaddress serverAddr;

    char localhostname[256];

    if (Client->numConnected>0) clientDisconnect(Client);//if we're already connected, disconnect
        //flushEventQueue(Client);
	//SDLNet_ResolveHost(&serverAddr, server, port);
        //Client->syncFreq = 2;

	/*if (Client->socket = SDLNet_TCP_Open(&serverAddr)) {
		Client->socketSet = SDLNet_AllocSocketSet(1);
		SDLNet_TCP_AddSocket(Client->socketSet, Client->socket);
		Client->isConnected = 1;
		Client->isWaitingForServer = 1;
	}*/
    IPaddress myAddr;
    JoinRequest *packet = ((JoinRequest*)&(Client->joinState.packet));
    if(SDLNet_ResolveHost(&(Client->joinState.host), server, port)==0) {
        if(gethostname(localhostname,256)==0 && 
                SDLNet_ResolveHost(&myAddr, localhostname, SERVER_PORT)==0) {
            //SDLNet_Write16(myAddr.port,Client->packet->data+2);
            //SDLNet_Write32(myAddr.host,Client->packet->data+4);
            //port & host should already be in network byte order
            packet->client.port=myAddr.port;
            packet->client.host=myAddr.host;
        }
        else
            memset(&(packet->client), 0, sizeof(IPaddress));

        SDLNet_Write16(FRAME_JOINREQUEST,&(packet->eID));

        Client->joinState.state=waiting;
        Client->eventQueue[Client->numQueued]->evt=EVENT_JOIN;
        Client->eventQueue[Client->numQueued]->time=Client->frameCounter;
        addEventToQueue(Client);
        return 1;
    }
	return 0;
}

void clientDisconnect(MupenClient *Client) {
       if (Client->numConnected>0) {
          printf("[Netplay] Disconnected from server.\n");
          osd_new_message(OSD_BOTTOM_LEFT, (void *)tr("You have disconnected from the server."));
          SDLNet_FreeSocketSet(Client->socketSet);
          SDLNet_UDP_Close(Client->socket);
          Client->numConnected = 0;
          flushEventQueue(Client);
       }
}

int clientSendMessage(MupenClient *Client) {
  /*if (msg) {
    if (Client->numConnected>1) {
	//if (SDLNet_TCP_Send(Client->socket, msg, sizeof(NetMessage)) != sizeof(NetMessage)) // If there is a problem sending then disconnect
	  //clientDisconnect(Client);
    }
  }*/
}

void clientProcessFrame(MupenClient *Client) {
    FrameChunk* curChunk = (FrameChunk*) Client->packet->data;
    int len=Client->packet->len-sizeof(Frame);
    int player=curChunk->header.peer;
    unsigned int frame=SDLNet_Read16(&(curChunk->header.eID));
    curChunk=((void *)curChunk)+sizeof(Frame);
    while(len>0) {
        switch(curChunk->type) {
          case CHUNK_INPUT:
            Client->playerEvent[frame%FRAME_BUFFER_LENGTH][curChunk->input.player].timer=frame;
            Client->playerEvent[frame%FRAME_BUFFER_LENGTH][curChunk->input.player].value=curChunk->input.buttons.Value;
            Client->playerEvent[frame%FRAME_BUFFER_LENGTH][curChunk->input.player].control=((curChunk->input.buttons.Value & 0x8000) != 0);
            //fprintf(stderr,"chunk player: %d frame: %d data: %08x\n",curChunk->input.player,frame,curChunk->input.buttons.Value);
            len-=sizeof(InputChunk);
          break;
          default:
            fprintf(stderr,"Invalid Frame received!\n");
        }
    }
}

void clientSendFrame(MupenClient *Client) {
    int i;
    FrameChunk *chunk = (FrameChunk*) Client->packet->data;

    for(i=0; i<Client->numConnected-1;i++){
        int curID=sourceID(Client->myID, i);
        Client->packet->address=Client->player[curID].address;
        Client->packet->len=sizeof(Frame);
        SDLNet_Write16((Client->frameCounter/VI_PER_FRAME)&FRAME_MASK,&(chunk->header.eID));
        chunk->header.peer=Client->myID;
        chunk->header.lag=Client->lag[curID];
        chunk = (FrameChunk*)(((char*)chunk)+sizeof(Frame));

        chunk->input.type = CHUNK_INPUT;
        chunk->input.player = Client->myID;
        NetPlayerUpdate *update = &(Client->playerEvent[(Client->frameCounter/VI_PER_FRAME)%FRAME_BUFFER_LENGTH][Client->myID]);
        if(update->timer!=(Client->frameCounter)/VI_PER_FRAME) {
            chunk->input.buttons.Value = (0x8000 * Client->startEvt);
            fprintf(stderr,"[Netplay] Sending BAD event: %08X u->timer: %d fC: %d\n",chunk->input.buttons.Value,update->timer,(Client->frameCounter)/VI_PER_FRAME);
        }
        else {
            chunk->input.buttons.Value = update->value | (0x8000 * Client->startEvt);
            //fprintf(stderr,"[NETPLAY]Sending event: %08X u->timer: %d fC: %d\n",chunk->input.buttons.Value,update->timer,(Client->frameCounter)/VI_PER_FRAME);
        }
        if(Client->startEvt) {
            Client->playerEvent[((Client->frameCounter/VI_PER_FRAME)&FRAME_MASK)%FRAME_BUFFER_LENGTH][Client->myID].control=1;
            Client->playerEvent[((Client->frameCounter/VI_PER_FRAME)&FRAME_MASK)%FRAME_BUFFER_LENGTH][Client->myID].timer=(Client->frameCounter/VI_PER_FRAME)&FRAME_MASK;
            Client->startEvt=0;
        }
        Client->packet->len+=sizeof(InputChunk);

//        fprintf(stderr,"send sync %d.%d.%d.%d:%d\n",GET_IP(Client->packet->address.host), GET_PORT(Client->packet->address.port));
        if (SDLNet_UDP_Send(Client->socket, -1, Client->packet) == 0) {
            printf("[Netplay] clientSendFrame(): Failure on SDLNet_UDP_Send.\n");
            clientDisconnect(Client);
        }
    }

}

void clientProcessMessages(MupenClient *Client) {
    char osdString[64];
    int n, playerNumber, adjust;
    Uint16 frame_ready_id = FRAME_PUNCH;

    //if (!(Client->isListening))
    //    return; // exit now if the client isnt' connected

    SDLNet_CheckSockets(Client->socketSet, 0);
    while (SDLNet_SocketReady(Client->socket)) {
        n = SDLNet_UDP_Recv(Client->socket, Client->packet);
        //n = SDLNet_TCP_Recv(Client->socket, &incomingMessage, sizeof(NetMessage));
        if (n < 0) {
            clientDisconnect(Client);
            return;
        }
	    if(Client->packet->len >= 2) {
            Uint16 frameID=SDLNet_Read16(Client->packet->data);
            switch(frameID) {

              /*
                  The master server will send a FRAME_PUNCHREQUEST packet to a p2p client which is
                  running a game and waiting for other players to connect.  The FRAME_PUNCHREQUEST
                  packet will contain the address of another peer that is trying to connect.
                  We will send a FRAME_PUNCH packet to that peer to create a NAT entry that
                  will allow that peer to communicate with us.

                  The peer which is trying to connect will also send a FRAME_PUNCHREQUEST packet
                  in the event that it doesn't receive the first FRAME_PUNCH packet sent in
                  response to the FRAME_PUNCHREQUEST packet sent from the master server.
              */

              case FRAME_PUNCHREQUEST:
                // address will be in network order, memcpy is ok
                memcpy(&(Client->packet->address.host), Client->packet->data + 2, 4);
                memcpy(&(Client->packet->address.port), Client->packet->data + 6, 4);

                // Send back FRAME_READY packet to specified address
                memcpy(Client->packet->data, &frame_ready_id, 2);
                Client->packet->len = 2;
                if (SDLNet_UDP_Send(Client->socket, -1, Client->packet) == 0) {
                    printf("[Netplay] FRAME_PUNCH: Error sending packet to %d.%d.%d.%d.\n", GET_IP(Client->packet->address.host));
                }
                break;

              case FRAME_PUNCH:
                // Should respond with status request to sender

                break;

              case FRAME_JOINREQUEST:
                if(Client->packet->len == sizeof(JoinRequest)) {
                    JoinRequest *packet=((JoinRequest*)Client->packet->data);
                    Uint16 port=packet->client.port;
                    Uint32 address=packet->client.host;
                    fprintf(stderr,"[NETPLAY]Received JoinRequest from %d.%d.%d.%d:%d (%d.%d.%d.%d:%d)", GET_IP(address),GET_PORT(port),
                         GET_IP(Client->packet->address.host), GET_PORT(Client->packet->address.port));
                    /*if(Client->joinState.state!=enabled)
                        fprintf(stderr,"... Ignoring\n");
                    else {
                        fprintf("... Responding\n")
                        Client->joinState.state=servicing;
                        ISSUE JOINS, 
                    }*/
                    if(Client->numConnected==0) {
                        Client->myID=0;
                        Client->numConnected++;
                        //send a JOIN response
                        Client->packet->len=16;
                        SDLNet_Write16(FRAME_JOIN, Client->packet->data);
                        SDLNet_Write16(Client->frameCounter, Client->packet->data+2);
                        SDLNet_Write32(Client->numConnected, Client->packet->data+4);
                        Client->player[Client->numConnected].address=Client->packet->address;
                        Client->numConnected++;
                        SDLNet_Write32(Client->numConnected, Client->packet->data+8);
                        SDLNet_Write32(Client->myID, Client->packet->data+12);
                        SDLNet_UDP_Send(Client->socket, -1, Client->packet);

                        Client->eventQueue[Client->numQueued]->evt=EVENT_INPUT;
                        Client->eventQueue[Client->numQueued]->time=Client->frameCounter+Client->inputDelay;
                        addEventToQueue(Client);
                    }
                }
                else
                    fprintf(stderr,"[NETPLAY]Received invalid length JoinRequest from %08X:%d %d bytes\n",
                        Client->packet->address, Client->packet->address.port, Client->packet->len);
                break;

              case FRAME_JOIN:
                if(Client->packet->len == 16) {
                    Client->joinState.state=enabled;
                    Client->frameCounter = SDLNet_Read16(Client->packet->data+2);
                    Client->myID = SDLNet_Read32(Client->packet->data+4);
                    Client->numConnected = SDLNet_Read32(Client->packet->data+8);
                    Uint32 remoteID = SDLNet_Read32(Client->packet->data+12);

                    Client->player[remoteID].address=Client->packet->address;

                    Client->eventQueue[Client->numQueued]->evt=EVENT_INPUT;
                    Client->eventQueue[Client->numQueued]->time=Client->frameCounter+Client->inputDelay+VI_PER_FRAME;
                    addEventToQueue(Client);


                    fprintf(stderr,"[NETPLAY]Received Join from %d.%d.%d.%d:%d\n",
                        GET_IP(Client->packet->address.host), GET_PORT(Client->packet->address.port));
                    //TODO: must connect to other peers as well!
                }
                else
                    fprintf(stderr,"[NETPLAY]Received invalid length Join reply from %08X:%d\n",
                        Client->packet->address, Client->packet->address.port);
                break;
              default://We likely have a frame
                if(frameID<=FRAME_MASK) {
                    int diff = frameDelta(Client, frameID * VI_PER_FRAME);
                    int absdiff=diff;
                    if(absdiff < 0) absdiff*=-1;
                    if((absdiff/VI_PER_FRAME) < (FRAME_BUFFER_LENGTH/2)-1) {
                        //NetPlayerUpdate* frEvt=Client->playerevent[frameID%FRAME_BUFFER_LENGTH];
                        Uint8 host = Client->packet->data[2];
                        char lag = Client->packet->data[3];
                        int curID = sourceID(Client->myID,0);
                        Client->lag[host] = ((((int)Client->lag[host])*3)/4) + diff;
                        Client->lag_local[host] = lag;
                        adjust = (lag - Client->lag[host])/4;
                        fprintf(stderr,"Host %d lag time local: %d remote: %d adjust: %d diff: %d\n",host,Client->lag[host],lag,adjust,diff);
                        setSpeed(100+(adjust));
                        clientProcessFrame(Client);
                    } else
                        fprintf(stderr,"[NETPLAY]Discarded packet for frame %d (it's currently frame %d)\n",
                            frameID, (Client->frameCounter/VI_PER_FRAME));
                    
                } else {
                    fprintf(stderr,"[NETPLAY]Unknown Frame header: %04X len:%d\n",
                        frameID,Client->packet->len);
                }
            }
        }
          //while(decodePacket(MupenClient *Client, &incomingMessage)) {
          /*if (n == sizeof(NetMessage)) {
                playerNumber = incomingMessage.genEvent.player;
		switch (incomingMessage.type) {
			case NETMSG_EVENT:
				if (incomingMessage.genEvent.timer >= Client->frameCounter) {
					addEventToQueue(Client, incomingMessage);
				}
				else {
                                   printf("[Netplay] DESYNC: Recieved a late event for frame %d (currently frame %d).\n", incomingMessage.genEvent.timer, Client->frameCounter);
                                   outgoingMessage.type = NETMSG_DESYNC;
                                   clientSendMessage(Client, &outgoingMessage);
                                }
			break;
                        case NETMSG_DESYNC:
				sprintf(osdString, "Player %d has desynchronized!", playerNumber + 1);
                                printf("[Netplay] %s\n", osdString);
				osd_new_message(OSD_BOTTOM_LEFT, (void *)tr(osdString));
                        break;
                        case NETMSG_SYNC:
                                //printf("[Netplay] Recieved sync message (End of frame %d)\n", incomingMessage.genEvent.timer);
				Client->lastSync = incomingMessage.genEvent.timer;
                        break;
			case NETMSG_PLAYERQUIT:
				sprintf(osdString, "Player %d has disconnected.", playerNumber + 1);
                                printf("[Netplay] %s\n", osdString);
				osd_new_message(OSD_BOTTOM_LEFT, (void *)tr(osdString));
			break;
			case NETMSG_PING:
                                clientSendMessage(Client, &incomingMessage);
                                printf("[Netplay] Received ping, responding.\n");
			break;
			default:
                                printf("[Netplay] Received an unrecognized message from the server.\n");
			break;
		}
	  //}*/
    }
}

void clientSendButtons(MupenClient *Client, int control, DWORD value) {
    if(control==0) {//will only allow 1 player per host, must be changed
        int bufInd=((Client->frameCounter/VI_PER_FRAME)+1) % FRAME_BUFFER_LENGTH;
        Client->playerEvent[bufInd][Client->myID].timer = (Client->frameCounter/VI_PER_FRAME)+1;
        Client->playerEvent[bufInd][Client->myID].value = value;
    }
}


void clientLoadPacket(MupenClient *Client, void *data, int len) {
    int i;

    Client->packet->len=len;
    for(i=0;i<len;i++)
        ((unsigned char*)(Client->packet->data))[i]=((unsigned char*)data)[i];
}

/* =======================================================================================

	The following functions are used to process the event queue 

   =======================================================================================
*/

// processEventQueue() : Process the events in the queue, if necessary.
// returns 1 if successful, or 0 if it doesn't have all the needed packets to continue
int processEventQueue(MupenClient *Client) {
    int i;
    while((Client->numQueued > 0) && (Client->eventQueue[0]->time <= Client->frameCounter)) {
        //fprintf(stderr,"B q-%d t-%d c-%d\n",Client->numQueued,Client->eventQueue[0]->time,Client->frameCounter);
        switch(Client->eventQueue[0]->evt) {
          case EVENT_JOIN:
            if (Client->joinState.state==waiting) {
                Client->packet->address=Client->joinState.host;
                clientLoadPacket(Client,&(Client->joinState.packet),sizeof(JoinRequest));
                SDLNet_UDP_Send(Client->socket, -1, Client->packet);
                Client->eventQueue[0]->time=Client->frameCounter+QUEUE_JOIN_DELAY;
                heapifyEventQueue(Client,0);
                fprintf(stderr,"[NETPLAY] Issuing join request to %d.%d.%d.%d:%d\n",
                    GET_IP(Client->joinState.host.host),GET_PORT(Client->joinState.host.port));
            }
            else if (Client->joinState.state==disabled) {
                Client->joinState.state=enabled;
            } 
            else if (Client->joinState.state==enabled)
                popEventQueue(Client);
          break;
          case EVENT_INPUT:
            for(i=0; i<Client->numConnected;i++) {
                int frame=Client->frameCounter-Client->inputDelay;
                NetPlayerUpdate *curUpdate=&(Client->playerEvent[(frame/VI_PER_FRAME) % FRAME_BUFFER_LENGTH][i]);
                if(curUpdate->control==1)
                    fprintf(stderr,"Unpause event frame %d, player %d, %d (%d)\n", curUpdate->timer, i, (frame/VI_PER_FRAME), (frame/VI_PER_FRAME) % FRAME_BUFFER_LENGTH);
                if(curUpdate->timer!=frame/VI_PER_FRAME) {
                    if(i!=Client->myID) {
                       fprintf(stderr,"[NETPLAY] Out of sync at frame %d (%d) player: %d\n", frame/VI_PER_FRAME, curUpdate->timer, i);
                        return 0;
                    }
                }
                else {
                    if(curUpdate->control==1) {
                        curUpdate->control=0;
                        rompause=!rompause;
                        printf("Unpause event frame %d\n",frame);
                    }
                    Client->playerKeys[i].Value=curUpdate->value;
//if(curUpdate->value != 0) fprintf(stderr,"curUpdate->value = %08X :%d :%d\n", curUpdate->value, i, Client->frameCounter);
                }
            Client->eventQueue[0]->time=Client->frameCounter+VI_PER_FRAME;
            heapifyEventQueue(Client,0);
            }
          break;
          default:
            fprintf(stderr,"[NETPLAY] Unexpected Event popped off of queue ID:%02x\n", Client->eventQueue[0]->evt);
          break;
        }
    }
    return 1;
  /*if (Client->eventQueue) {
    while (Client->eventQueue->timer < Client->frameCounter) {
       printf("[Netplay] DESYNC: Missed event for frame %d (currently frame %d).\n", Client->eventQueue->timer, Client->frameCounter);
       outgoingMessage.type = NETMSG_DESYNC;
       clientSendMessage(Client, &outgoingMessage);
       popEventQueue(Client);
    }
    while ((Client->eventQueue) && (Client->eventQueue->timer == Client->frameCounter)) {
	switch (Client->eventQueue->type) {
          case EVENT_BUTTON: 
            if (Client->eventQueue->controller < MAX_CLIENTS) {
                Client->playerKeys[Client->eventQueue->controller].Value = Client->eventQueue->value;
                printf("[Netplay] Player %d: button lag %d ms.\n", Client->eventQueue->controller + 1, gettimeofday_msec()-Client->eventQueue->time_stamp);
            } else {
                printf("[Netplay] Received message from player %d (out of range).\n", Client->eventQueue->controller);
            }
            break;
       }
       popEventQueue(Client);
    }
  }*/
}

void swapQueueItems(MupenClient *Client, int a, int b) {
    NetEvent* temp = Client->eventQueue[a];
    (Client->eventQueue)[a]=Client->eventQueue[b];
    Client->eventQueue[b]=temp;
}

void heapifyEventQueue(MupenClient *Client,unsigned int elem){
    int left=elem*2+1;
    int right=elem*2+2;
    if(right < Client->numQueued) {
        if(Client->eventQueue[right]->time <= Client->eventQueue[left]->time) {
            if(Client->eventQueue[right]->time < Client->eventQueue[elem]->time) {
                swapQueueItems(Client,right,elem);
                heapifyEventQueue(Client,right);
                return;
            }
            else
                return;
        }
    }
    if (left < Client->numQueued){
        if(Client->eventQueue[left]->time < Client->eventQueue[elem]->time) {
            swapQueueItems(Client,left,elem);
            heapifyEventQueue(Client,left);
            return;
        }
        else
            return;
    }
}

// addEventToQueue() : Add a new button event to the button event queue.
void addEventToQueue(MupenClient *Client) {

    swapQueueItems(Client,0,Client->numQueued);

    Client->numQueued++;

    heapifyEventQueue(Client,0);

    if(Client->numQueued>QUEUE_HEAP_LEN) {
        fprintf(stderr,"[NETPLAY] Queue Overflow!\n");
        exit(1);
    }
}

// popEventQueue() : Remove the event in the front of the queue.
void popEventQueue(MupenClient *Client) {

    if((--Client->numQueued)>0) {
        swapQueueItems(Client,0,Client->numQueued);

        heapifyEventQueue(Client,0);
    }
    if(Client->numQueued>QUEUE_HEAP_LEN) {
        fprintf(stderr,"[NETPLAY] Queue Underflow!\n");
        exit(1);
    }

}

void flushEventQueue(MupenClient *Client) {
    Client->numQueued=0;
}


