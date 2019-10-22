#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <zlib.h>
#include "functions.h"
#include "functions.c"



int main (int argc, char **argv){

    printf("Starting ... \n");

  	crc = crc32(0L,Z_NULL,0);
	//crc needs to be initialised at each startup

	argReader(argc,argv);

	printf("Args processed \n");
    printf("address %s , port %d  \n" , address , port);



    //init part
    int sock = socket(AF_INET6,SOCK_DGRAM,0);
    if(sock == -1 ){printf("Erreur lors de la création des sockets.\n");}

    struct sockaddr_in6 peer_addr;                      // allocate the address on the stack
    memset(&peer_addr, 0, sizeof(peer_addr));           // fill the address with 0-bytes to avoid garbage-values
    peer_addr.sin6_family = AF_INET6;                   // indicate that the address is an IPv6 address
    peer_addr.sin6_port = htons(port);                 // indicate that the programm is running on port 55555
    inet_pton(AF_INET6, address, &peer_addr.sin6_addr);   // indicate that the program is running on the computer identified by the ::1 IPv6 address



    //sending part
    ssize_t sent=0;



/*
	Buffer buff;
	buff.content = calloc(64,sizeof(char));
	if(buff.content==NULL){printf("Le malloc a échoué\n");}
*/

//printf("Size of content: %d\n", (int)sizeof(*(buff.content)));
    char * payload[512] = {0};

    int payLen = getPayload((char*)payload, file, 512);

    int L = 0;
    if(payLen >= 512){
        L = 1;
    }

    Paquet p = packetConstructor(1,0,1,L,payLen,0,3);



    int shift = 0 ;
    if(L == 0){
        shift = 1;
    }
    char * finalbuffer = malloc(sizeof(char)*(payLen+16-shift));
//	finalbuffer = packetGenerator(p,payload,payLen);


    memcpy(finalbuffer,packetGenerator(p,(char*)payload,payLen),sizeof(char)*(payLen+16-shift));



    sent = sendto(sock,finalbuffer,payLen+16-shift,0,(const struct sockaddr *)&peer_addr, sizeof(peer_addr));



    char * payload2[512] = {0};

    int payLen2 = getPayload((char*)payload2, file, 512);

    int L2 = 0;
    if(payLen2 >= 512){
        L2 = 1;
    }
    int shift2 = 0 ;
    if(L2 == 0){
        shift2 = 1;
    }

    Paquet p2 = packetConstructor(1,0,0,0,0,1,32);


    char * finalbuffer2 = malloc(sizeof(char)*(12-shift2));
//	finalbuffer = packetGenerator(p,payload,payLen);

    memcpy(finalbuffer2,packetGenerator(p2,NULL,0),sizeof(char)*(16-shift2));



    sent = sendto(sock,finalbuffer2,12-shift2,0,(const struct sockaddr *)&peer_addr, sizeof(peer_addr));

    return 0;}
