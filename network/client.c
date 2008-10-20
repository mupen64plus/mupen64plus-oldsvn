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
#include <zlib.h>
#include "network.h"
#include "../main/md5.h"
#include "../r4300/r4300.h"
#include "../r4300/macros.h"

int         g_Game_ID;

int clientInitialize(MupenClient *Client) {
    int i;
    memset(Client, 0, sizeof(MupenClient));

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
    int late = curChunk->header.late;
    if(frame >= Client->frameCounter + FRAME_BUFFER_LENGTH - Client->inputDelay) {
        fprintf(stderr,"[NETPLAY]Discarded packet for frame %d (it's currently frame %d)\n", frame, (Client->frameCounter/VI_PER_FRAME));
        return;
    }

    if((Client->lag[player]-Client->lag_local[player])>0)
        Client->timeStamp+=(Client->lag[player]-Client->lag_local[player]);

    Client->lag_local[player] = curChunk->header.lag;
    int delay=(gettimeofday_msec() - Client->timeStamp - (late*FRAME_LATENCY_SCALE)) - (((((signed int)frame) * VI_PER_FRAME) - ((signed int)Client->frameStamp) ) * 16.666666);
    Client->lag[player] = ( (((int)Client->lag[player]) * 3 + (delay / FRAME_LATENCY_SCALE)) / 4);

    //now move curChunk pointer to end of frame header
    curChunk=((void *)curChunk)+sizeof(Frame);
    static int prevtime=0;
    int time;
    //fprintf(stderr,"Host %d-%d lag time local: %d remote: %d\n", player, frame, Client->lag[player], Client->lag_local[player]);
#ifdef DUMP_TIMING_DATA
    //printf("%d %d %d %d\n",gettimeofday_msec(),Client->timems,rompause,timervalue);
    int offset=(Client->lag[player]-Client->lag_local[player])*FRAME_LATENCY_SCALE;
    time = gettimeofday_msec();
    if(prevtime==0)
        prevtime = time;
    double framedelay = ((gettimeofday_msec()-Client->timeStamp) / 16.6666666+Client->frameStamp)/4-frame;
    fprintf(stderr,"TIMING(%d,%f): %d stampfme: %d fme: %d delay %d diff %f ts %d\n", frame,Client->frameCounter/4.0, gettimeofday_msec(), Client->frameStamp, frame*VI_PER_FRAME, delay,framedelay,Client->timeStamp);
    gzprintf(Client->timeDump,"%f,%d,%d,%d,%d,%d,%d,%f,%d\n",Client->frameCounter / (1.0f*VI_PER_FRAME),player,time,time-prevtime,late*FRAME_LATENCY_SCALE,Client->lag[player],Client->lag_local[player],framedelay,offset);
    prevtime=time;
#endif //DUMP_TIMING_DATA
    while(len>0) {
        switch(curChunk->type) {
          case CHUNK_INPUT:
            Client->playerEvent[frame%FRAME_BUFFER_LENGTH][curChunk->input.player].timer=frame;
            Client->playerEvent[frame%FRAME_BUFFER_LENGTH][curChunk->input.player].value=curChunk->input.buttons.Value;
            Client->playerEvent[frame%FRAME_BUFFER_LENGTH][curChunk->input.player].control=((curChunk->input.buttons.Value & 0x8000) != 0);
            Client->playerEvent[frame%FRAME_BUFFER_LENGTH][curChunk->input.player].timems = gettimeofday_msec();
            //fprintf(stderr,"chunk player: %d frame: %d data: %08x control %d\n", curChunk->input.player,frame,curChunk->input.buttons.Value,curChunk->input.player);
            len-=sizeof(InputChunk);
          break;
          default:
            printf("NETPLAY: Invalid Frame received!\n");
        }
    }
}

void clientSendFrame(MupenClient *Client) {
    int i;

    FrameChunk *chunk = (FrameChunk*) Client->packet->data;
    int delay=(16 + gettimeofday_msec() - Client->timeStamp) - ((Client->frameCounter - Client->frameStamp)*16.66666666);
fprintf(stderr,"Send %d, delta %dms\n",Client->frameCounter,delay);
#ifdef DUMP_TIMING_DATA
    //gzprintf(Client->timeDump,"%f,%d,%d,%d,%d,%f,%d\n",Client->frameCounter / (1.0f*VI_PER_FRAME),Client->myID,gettimeofday_msec(),Client->lag[Client->myID],Client->lag_local[Client->myID],0.0f,0);
#endif //DUMP_TIMING_DATA

    // Get input here, this is temporary but should resolve some issues
    BUTTONS Keys;
#ifdef VCR_SUPPORT
        VCR_getKeys(0, &Keys);
#else
        getKeys(0, &Keys);
#endif

    int bufInd=(Client->frameCounter/VI_PER_FRAME) % FRAME_BUFFER_LENGTH;
    NetPlayerUpdate *localUpdate = &(Client->playerEvent[bufInd][Client->myID]);
    localUpdate->timer = (Client->frameCounter/VI_PER_FRAME);
    localUpdate->value = Keys.Value & 0xFFFF7FFF;
    localUpdate->timems = gettimeofday_msec();
    for(i=0; i<Client->numConnected-1;i++){
        int curID=sourceID(Client->myID, i);
        Client->packet->address=Client->player[curID].address;
        Client->packet->len=sizeof(Frame);
        SDLNet_Write16((Client->frameCounter/VI_PER_FRAME)&FRAME_MASK,&(chunk->header.eID));
        chunk->header.peer=Client->myID;
        chunk->header.lag=Client->lag[curID];
        chunk->header.late=delay/FRAME_LATENCY_SCALE;
        chunk = (FrameChunk*)(((char*)chunk)+sizeof(Frame));

        chunk->input.type = CHUNK_INPUT;
        chunk->input.player = Client->myID;

        chunk->input.buttons.Value = (localUpdate->value & 0xFFFF7FFF) | (0x8000 * Client->startEvt);
        //fprintf(stderr,"[NETPLAY]Sending event: %08X u->timer: %d fC: %d sE: %d\n",chunk->input.buttons.Value,update->timer,(Client->frameCounter)/VI_PER_FRAME,Client-);
        
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
    Uint32 host;
    Uint16 port;

    //if (!(Client->isListening))
    //    return; // exit now if the client isnt' connected
//fprintf(stderr,"process msgs %dms\n",gettimeofday_msec());
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

              // FRAME_MASTER =======================================================================
              case FRAME_MASTER:

                // ** Response to OPEN_GAME Request
                if (Client->packet->data[2] == GAME_DESC) {
                   g_Game_ID = SDLNet_Read16(Client->packet->data + 3);
                   printf("[Netplay] Received response to OPEN_GAME request, gid %d\n", g_Game_ID);
                }
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

                // ** Need a NAT punch
                else if (Client->packet->data[2] == PUNCH_REQ) {
                  memcpy(&host, Client->packet->data + 3, 4);
                  memcpy(&port, Client->packet->data + 7, 4);

                  if ((host != 0) && (Client->packet->address.host == Client->masterServer->host)) {
                    Client->packet->address.host = host;
                    Client->packet->address.port = port;
                  }

                  memcpy(Client->packet->data, &frame_ready_id, 2);
                  Client->packet->len = 2;

                  if (SDLNet_UDP_Send(Client->socket, -1, Client->packet) == 0) {
                    printf("[Netplay] Error sending packet to %d.%d.%d.%d.\n", GET_IP(Client->packet->address.host));
                  } else {
                    printf("[Netplay] NAT punch packet sent to %d.%d.%d.%d.\n", GET_IP(Client->packet->address.host));
                  }
                } 
                else {
                   printf("[Netplay] Unexpected packet received from master server (type %d).\n", Client->packet->data[2]);
                }
                break;

              case FRAME_PUNCH:
                // Should respond with status request to sender

                break;

              // FRAME_JOINREQUEST =================================================================
              case FRAME_JOINREQUEST:
                if(Client->packet->len == sizeof(JoinRequest)) {
                    JoinRequest *packet=((JoinRequest*)Client->packet->data);
                    Uint16 port=packet->client.port;
                    Uint32 address=packet->client.host;
                    printf("[NETPLAY] Received JoinRequest from %d.%d.%d.%d:%d\n",
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
                        Client->eventQueue[Client->numQueued]->time=Client->frameCounter+Client->inputDelay+VI_PER_FRAME;
                        addEventToQueue(Client);
                    }
                }
                else
                    fprintf(stderr,"[NETPLAY]Received invalid length JoinRequest from %08X:%d %d bytes\n",
                        Client->packet->address, Client->packet->address.port, Client->packet->len);
                break;

              // FRAME_JOIN ========================================================================
              case FRAME_JOIN:
                if(Client->packet->len == 16) {
                    Client->joinState.state=enabled;
                    Client->frameCounter = SDLNet_Read16(Client->packet->data+2);
                    Client->myID = SDLNet_Read32(Client->packet->data+4);
                    Client->numConnected = SDLNet_Read32(Client->packet->data+8);
                    Uint32 remoteID = SDLNet_Read32(Client->packet->data+12);
                    Client->player[remoteID].address=Client->packet->address;

                    Client->frameStamp=Client->frameCounter+1;
                    Client->timeStamp=gettimeofday_msec();

                    flushEventQueue(Client);
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
                    clientProcessFrame(Client);
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

/*void clientSendButtons(MupenClient *Client, int control, DWORD value) {
    if(control==0) {//will only allow 1 player per host, must be changed
        int bufInd=((Client->frameCounter/VI_PER_FRAME)+1) % FRAME_BUFFER_LENGTH;
        Client->playerEvent[bufInd][Client->myID].timer = (Client->frameCounter/VI_PER_FRAME)+1;
        Client->playerEvent[bufInd][Client->myID].value = value;
    }
}*/


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
    int i, frame;
//    for(i=0; i<Client->numQueued; i++)
//        fprintf(stderr,"%d ",Client->eventQueue[i]->time);
//    fprintf(stderr,"\n");
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
            frame=Client->frameCounter-Client->inputDelay;
           //gzprintf(Client->inputDump,"Input Event frame:%f event:%f cpu: %d\n",Client->frameCounter/(1.0f*VI_PER_FRAME),frame/(1.0f*VI_PER_FRAME),Count);

            for(i=0; i<Client->numConnected;i++) {
                NetPlayerUpdate *curUpdate=&(Client->playerEvent[(frame/VI_PER_FRAME) % FRAME_BUFFER_LENGTH][i]);
                if(curUpdate->control==1)
                    fprintf(stderr,"Unpause event frame %d, player %d\n", frame, i);
                if(curUpdate->timer!=frame/VI_PER_FRAME) {
                    if(i!=Client->myID) {
                        if(rompause==0) {
                            fprintf(stderr,"[NETPLAY] Out of sync at frame %f (%d) player: %d\n", frame/(1.0f*VI_PER_FRAME), curUpdate->timer, i);
                            rompause=1;
                            //Client->timems=gettimeofday_msec()-Client->timems;
                        }
                        //fprintf(stderr,"[NETPLAY] Skip cycle %d != %d\n", curUpdate->timer,frame/VI_PER_FRAME);
                        return 0;
                    }
                }
                else {
                    if(curUpdate->control==1) {
                        curUpdate->control=0;
/*                        if(rompause)
                            Client->timems=gettimeofday_msec()-Client->timems;
                        else
                            Client->timems=gettimeofday_msec()-Client->timems;
*/
                        rompause=!rompause;
                        printf("Unpause event frame %d\n",frame);
                        if(Client->emuState==paused) {
                            Client->emuState=running;
                        }
                        else {
                            Client->emuState=paused;
                        }
                    }
                    if(rompause==1 && Client->emuState==running) {
                        rompause=0;
                        //Client->timems=gettimeofday_msec()-Client->timems;
                    }
                    Client->playerKeys[i].Value=curUpdate->value;
//if(curUpdate->value != 0) fprintf(stderr,"curUpdate->value = %08X :%d :%d\n", curUpdate->value, i, Client->frameCounter);
                }
            }
            Client->eventQueue[0]->time=Client->frameCounter+VI_PER_FRAME;
            heapifyEventQueue(Client,0);
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
//  note this function heapifies queue by bubbling down events due to come later
//    fprintf(stderr,"EQ:down %d\n",elem);

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
    //fprintf(stderr,"EQ:add\n");

    int elem=Client->numQueued++;

    if(Client->numQueued>QUEUE_HEAP_LEN) {
        fprintf(stderr,"[NETPLAY] Queue Overflow!\n");
        exit(1);
    }

    while(elem>0 && 
        Client->eventQueue[elem]->time > Client->eventQueue[(elem-1)/2]->time) {
        swapQueueItems(Client,elem,(elem-1)/2);
        elem=(elem-1)/2;
    }
}

// popEventQueue() : Remove the event in the front of the queue.
void popEventQueue(MupenClient *Client) {
//    fprintf(stderr,"EQ:pop\n");

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


