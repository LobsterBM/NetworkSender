#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/time.h>
#include <zlib.h>
#include "functions.h"
#include "functions.c"



char * address;
int port ;
FILE *file;
uint32_t crc ;
uint32_t crc2 ;




typedef struct Buffer Buffer;
typedef struct Paquet Paquet;



//fonctions définies
void display(Paquet p);
void structToBuff(Paquet p, Buffer *b);
void buffToStruct(Paquet *p, Buffer b);
int is_time_out(struct timeval t, int timeout);
//fin



int main (int argc, char **argv){

    printf("Starting ... \n");

  	crc = crc32(0L,Z_NULL,0);
	//crc needs to be initialised at each startup

	argReader(argc,argv);

	printf("Args processed \n");
    printf("address %s , port %d  \n" , address , port);



    //init part
    int sock = socket(AF_INET6,SOCK_DGRAM,0);
    if(sock == -1 ){fprintf(stderr,"Erreur lors de la création des sockets.\n");}

    struct sockaddr_in6 peer_addr;                      // allocate the address on the stack
    memset(&peer_addr, 0, sizeof(peer_addr));           // fill the address with 0-bytes to avoid garbage-values
    peer_addr.sin6_family = AF_INET6;                   // indicate that the address is an IPv6 address
    peer_addr.sin6_port = htons(port);                 // indicate that the programm is running on port 55555
    inet_pton(AF_INET6, address, &peer_addr.sin6_addr);   // indicate that the program is running on the computer identified by the ::1 IPv6 address


    struct sockaddr_in6 peer2_addr;
	socklen_t peer2_len = sizeof(peer2_addr);
    	
    //poll
    int fds_length=2;
    struct pollfd fds[fds_length];

    //truc genre window seqnum
    int window=2;
    int windowSlide=window;
    int timeout=1000;//millisec
    int timeoutPerso=1000;//microsec
    char **sendingBuffer[windowSlide];
    //char *receivBuffer[528];
    Paquet receivPacket;
    Buffer receivBuffer;
    int seqnumtab[2];//valeur doit valoir window
   	for(int i=0;i<windowSlide;i++){
   		seqnumtab[i]=-1;
   	   	}
    int seqnum=0;
    struct timeval timeSending[2];//same as just above
    int nfull=0;
    



    
    ssize_t sent=0;
    char * payload[512] = {0};
    //char * payloadTest[512] = "yolo.";

    int payLen = getPayload((char*)payload, file, 512);
    //int payLenTest = getPayload((char*)payloadTest, file, 512);


    while(payLen>0 || nfull>0){

    	//poll
    	fds[0].fd=sock;
		fds[0].events= 0;
		fds[0].events |= POLLIN;

		fds[1].fd=sock;
		fds[1].events = 0;
		fds[1].events |= POLLOUT;

		int pret = poll(fds,fds_length,timeout);


		if(pret>0 ){
				int found =0;
				//printf("%d\n",fds[1].events );
				if(fds[1].revents==POLLOUT && window>0){
					int num = seqnum++;
					int L = 0;
				    if(payLen >= 128){
				        L = 1;
				    }

				    Paquet p = packetConstructor(1,0,1,L,payLen,num,3);//type,TR,window,L,length,seqnum,timestamp

				    int shift = 0 ;
				    if(L == 0){
				        shift = 1;
				    }
				    

				    char * finalbuffer;
				    if(payLen>0){
				    	finalbuffer = malloc(sizeof(char)*(payLen+16-shift));
				    	//finalbuffer = packetGenerator(p,payload,payLen);
						memcpy(finalbuffer,packetGenerator(p,(char*)payload,payLen),sizeof(char)*(payLen+16-shift));
				    }
				    
				  	

				    for(int i=0;i<windowSlide;i++){
				    	//check si timeout
				    	if(seqnumtab[i]!=-1 && is_time_out(timeSending[i],timeoutPerso)){
				    		//fprintf(stderr,"timeout dépassé i:%d\n",i);
				    		gettimeofday(&timeSending[i],NULL);
				    		sent = sendto(sock,sendingBuffer[i],sizeof(*sendingBuffer[i]),0,(const struct sockaddr *)&peer_addr, sizeof(peer_addr));
				    		//sent = sendto(sock,sendingBuffer[i],payLen+16-shift,0,(const struct sockaddr *)&peer_addr, sizeof(peer_addr));
				    		if(sent==-1){fprintf(stderr,"fail to resend.\n");}
				    		}
				    	
				    	//check
				    	if(seqnumtab[i]==-1 && found==0 && payLen>0){
				    		seqnumtab[i]=num;
				    		sendingBuffer[i]=finalbuffer;
				    		gettimeofday(&timeSending[i],NULL);
				    		//printf("envoyé seqnum:%d,  i:%d\n",num,i);
				    		found=1;
				    		nfull++;//on rempli de un le seqnumtab
				    		}
				    	}
				    
				    if(payLen>0){
				    	//printf("envoyé seqnum:%d",num);
				    	sent = sendto(sock,finalbuffer,payLen+16-shift,0,(const struct sockaddr *)&peer_addr, sizeof(peer_addr));
				    }	
				    


				   // *payload =0;
				    //memcpy(payload,0, sizeof(char)*512);
				    payLen = 0;
				    payLen = getPayload((char*)payload, file, 512);
				   // printf("payload from file %d \n ", payLen);
				    window--;
					}
				
				else if(fds[0].events==1 && fds[0].revents==POLLIN){
					receivBuffer.content = calloc(528,sizeof(char));
					ssize_t reception = recvfrom(sock,receivBuffer.content,sizeof(receivBuffer.content),0,(struct sockaddr *)&peer2_addr,&peer2_len);
					//int seqnumReceiv = atoi((const char *)receivBuffer);
					buffToStruct(&receivPacket,receivBuffer);
					//display(receivPacket);
					//printf("ack %d\n", receivPacket.Seqnum);
					 for(int i=0;i<windowSlide;i++){
					 	//printf("vérif seqnumtab:%d et seqnumReceiv:%d\n",seqnumtab[i],seqnumReceiv );
				    	if(seqnumtab[i]==receivPacket.Seqnum){
				    		//printf("libération du buffer pour le seqnum:%d\n",receivPacket.Seqnum);
				    		seqnumtab[i]=-1;
				    		sendingBuffer[i]="\0";
				    		window++;
				    		nfull--;//on libère le seqnum
				    		break;
				    	}
				    }
				}
			



		}

	    if(payLen == 0){
	        sleep(0.1);
	        break;
	    }


    }



    //cloture
    char * payload2[512] = {0};

    //printf("eof packet sending");
    int payLen2 = getPayload((char*)payload2, file, 512);

    int L2 = 0;
    if(payLen2 >= 128){
        L2 = 1;
    }
    int shift2 = 0 ;
    if(L2 == 0){
        shift2 = 1;
    }

    Paquet p2 = packetConstructor(1,0,window,0,0,seqnum++,31);


    char * finalbuffer2 = malloc(sizeof(char)*(12-shift2));
//	finalbuffer = packetGenerator(p,payload,payLen);

    memcpy(finalbuffer2,packetGenerator(p2,NULL,0),sizeof(char)*(12-shift2));




    sent = sendto(sock,finalbuffer2,12-shift2,0,(const struct sockaddr *)&peer_addr, sizeof(peer_addr));

    printf("Success! \n");

    return 0;}




