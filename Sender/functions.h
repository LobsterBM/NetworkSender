#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <zlib.h>


int argReader(int argc, char *argv[]);

int getPayload(char * payload, FILE * file, int byteRead);





typedef struct Buffer Buffer;
typedef struct Paquet Paquet;


struct Paquet
{
	 unsigned int type : 2; //ack nack data
	 unsigned int TR : 1;
	 unsigned int window: 5;
	 unsigned int L : 1;
	 unsigned int length7: 7;
	 unsigned int length15: 15;
	 unsigned int Seqnum: 8;
	 unsigned int Timestamp: 32;

};

struct Buffer
{

	unsigned int c : 32;
	unsigned char *content;
};



char * packetGenerator ( Paquet p , char * payload , int payloadLen);

struct Paquet packetConstructor(unsigned int type ,unsigned int TR ,unsigned int window,unsigned int L ,unsigned int length ,unsigned int seqnum,unsigned int timestamp);


void structToBuff(Paquet p, Buffer *b);

void buffToStruct(Paquet *p, Buffer b);



void display(Paquet p);

#endif