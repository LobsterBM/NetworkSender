#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <zlib.h>


uint32_t crc;
uint32_t crc2;

typedef struct Paquet Paquet;
struct Paquet
{
	 unsigned int type : 2;
	 unsigned int TR : 1;
	 unsigned int window: 5;
	 unsigned int L : 1;
	 unsigned int length7: 7;
	 unsigned int length15: 15;
	 unsigned int Seqnum: 8;
	 unsigned int Timestamp: 32;
	
};


typedef struct Buffer Buffer;
struct Buffer
{
	
	unsigned int c : 32;
	unsigned char *content;
};

//fonctions définies
void display(Paquet p);
void structToBuff(Paquet p, Buffer *b);
void buffToStruct(Paquet *p, Buffer b);
int init_buff(char *buff, int buff_len);
struct Paquet packetConstructor(unsigned int type ,unsigned int TR ,unsigned int window,unsigned int L ,unsigned int length ,unsigned int seqnum,unsigned int timestamp);
char * packetGenerator( Paquet p , char * payload , int payloadLen);
//fin


int main (int argc, char **argv){


	crc = crc32(0L,Z_NULL,0);
	//init part
	int sock = socket(AF_INET6,SOCK_DGRAM,0);
	if(sock == -1 ){printf("Erreur lors de la création des sockets.\n");}

	struct sockaddr_in6 self_addr;                      // allocate the address on the stack
	memset(&self_addr, 0, sizeof(self_addr));           // fill the address with 0-bytes to avoid garbage-values
	self_addr.sin6_family = AF_INET6;                   // indicate that the address is an IPv6 address
	self_addr.sin6_port = htons(55555);                 // indicate that the programm is running on port 55555
	inet_pton(AF_INET6, "::1", &self_addr.sin6_addr);   // indicate that the program is running on the computer identified by the ::1 IPv6 address

	//receving part
	ssize_t buff_maxsize = 512;
	
	socklen_t self_addr_len = sizeof(self_addr);

	ssize_t bound = bind(sock,(const struct sockaddr*)&self_addr,self_addr_len);
	if(bound == -1){printf("Erreur lors de la liaison\n");}
	

	Paquet p,p2;
	int found=0;


	while(1){
	
		//buffers
		Buffer buff;
		buff.content = calloc(buff_maxsize,sizeof(char));
		if(buff.content==NULL){printf("fail calloc\n");}
		char * buffSending= calloc(12,sizeof(char));
		if(buffSending==NULL){printf("fail calloc\n");}

		//info reception
		struct sockaddr_in6 peer_addr;
		socklen_t peer_len = sizeof(peer_addr);
		ssize_t reception = recvfrom(sock,buff.content,buff_maxsize,0,(struct sockaddr *)&peer_addr,&peer_len);
		

		

		int eof=0;
		eof|=*(buff.content+1);
		if(eof!=0 ){
			

			buffToStruct(&p,buff);
			//display(p);
			//sprintf(buffSending,"%d",p.Seqnum);
			printf("found :%d\n",found);	
			p2 = packetConstructor(2,0,5,0,0,p.Seqnum,4);
			char *finalbuff =  malloc(sizeof(char)*11);
			memcpy(finalbuff,packetGenerator(p,NULL,0),sizeof(char)*11);
			if(found!=2){
				//ssize_t sent = sendto(sock,buffSending,sizeof(*buffSending),0,(const struct sockaddr *)&peer_addr, sizeof(peer_addr));
				ssize_t sent = sendto(sock,finalbuff,11,0,(const struct sockaddr *)&peer_addr, sizeof(peer_addr));
				if(sent==-1){printf("fail to send msg back.\n");}
				printf("ack de seqnum: %d envoyé\n", p.Seqnum);
			}
			found++;
			
		}
		else{printf("Dernier paquet envoyé.\n");}
	
		

		

	}

	

	printf("Server closed.\n");


return 0;}

struct Paquet packetConstructor(unsigned int type ,unsigned int TR ,unsigned int window,unsigned int L ,unsigned int length ,unsigned int seqnum,unsigned int timestamp){
    Paquet res ;

    res.type = type;
    res.TR = TR;
    res.window = window;
    res.L = L;
    res.length7 = length;
    res.length15 = length;
    res.Seqnum = seqnum;
    res.Timestamp = timestamp;

    return res;

}


char * packetGenerator( Paquet p , char * payload , int payloadLen){
    //TODO TR 1 case
    // shift in case of length 7 instead of 15
    int shift = 0;
    if(!(p.L)) shift = 1;

	char *packet = malloc(sizeof(char)*(16-shift+payloadLen));

    char* header[8-shift];

    Buffer headBuff ;
    headBuff.content = calloc(8-shift,sizeof(char));
    if(headBuff.content==NULL){printf("Le malloc a échoué\n");}
    structToBuff(p,&headBuff);
    //packet to buffer


    //header buffer to final packet buffer
    for(int i = 0 ; i < 8-shift ; i++){
         header[i] =  headBuff.content[i];
    }

    //copy header to final packet
    for(int i = 0 ; i < 8-shift ; i++){
        packet[i] =  header[i];
    }



    crc= 0;
    crc  = (uint32_t) crc32(crc , (Bytef *)(headBuff.content), 8-shift);
    //crc= 2214560385;
    uLong crccpy=crc;

    uint8_t tempcrc=0;
    for(int i = 0 ; i < 4 ;i++){
        tempcrc=crccpy;
        packet[11-shift-i] =tempcrc;
        crccpy=crccpy >>8;
    }
    //packet[12-shift] = crc;
    //payload[32] = crc;




    for (int i = 0 ; i < payloadLen; i++){
    	packet[i+12-shift] = payload[i];
    	//98 for 64 header and 32 crc
    }



    crc2 = 0;
    crc2  = (uint32_t) crc32(crc2 , (Bytef *)(payload), payloadLen);

    uLong crccpy2=0;
     crccpy2=crc2;

    uint8_t tempcrc2=0;
    for(int i = 0 ; i < 4 ;i++){
        tempcrc2=crccpy2;
        packet[payloadLen+16-1-shift-i] =tempcrc2;
        crccpy2=crccpy2 >>8;
    }

   // printf("\n crc 2 post add : %lu \n" , (uLong) packet[payloadLen+12-shift+1] );

    return packet;



}


//TODO check structure in binary for during execution
void structToBuff(Paquet p, Buffer *b){
	
	*((*b).content)  = 0;
	//TODO memeset  sur la longueur  su buffer a 0
	//printf("byte 1 step 1:%u\n",*((*b).content));

	*((*b).content)= *((*b).content) | p.type;

	//printf("byte 1 step 2:%u\n",*((*b).content) );
	
	*((*b).content)= *((*b).content) << 1;
	*((*b).content)= *((*b).content) | p.TR;
	//printf("byte 1 step 3:%u\n",*((*b).content) );
	
	*((*b).content) = *((*b).content)  << 5;
	*((*b).content)  = *((*b).content)  | p.window;
	//printf("byte 1 step 4:%u\n",*((*b).content) );//ok jusqu'ici

	//printf("byte 2 setp 1:%u\n",*((*b).content+1));
//	*((*b).content+1)  = *((*b).content+1) | p.L;
	//printf("byte 2 setp 2:%u\n",*((*b).content+1));
	
	//i => indice de décalage (nom biz)
	int i;

	if(p.L==0){
		//*((*b).content+1)  = *((*b).content+1) << 7;
		*((*b).content+1)  = *((*b).content+1) | p.length7;
		//printf("byte 2 setp 3:%u\n",*((*b).content+1));
		i=2;
	
	}
	else{

		i=3;
		//décomposition length15
		uint8_t l1 =0; // 2 bytes makin:g up length , so 2 bytes
		uint8_t l2 =0;
		l2 =  p.length15 | l2;
		p.length15 = p.length15 >> 8;
		l1= p.length15 | l1;
	//	printf("l1: %u\n",l1);
	//	printf("l2: %u\n",l2);

        l1 +=128;
   //     printf("l1 +128: %u\n",l1);
    //    printf(" \n here1:::%d" , *((*b).content+1));

        //encodage du length15
        *((*b).content+1)  = *((*b).content+1) | l1;
      //  printf(" \n here:::%d" , *((*b).content+1));
		*((*b).content+2)  = *((*b).content+2) | l2;

		
	}

		//encodage Seqnum
		*((*b).content+i)  = *((*b).content+i)  | p.Seqnum;
		//printf("byte seqnum setp 1:%u\n",*((*b).content+i));

		
		//décomposition  du timestamp
		uint8_t w =0;
		uint8_t x =0;
		uint8_t y =0;
		uint8_t z =0;

		z=p.Timestamp |z;
		p.Timestamp = p.Timestamp >> 8;
		//printf("Décompo z = %u\n",z );
		y=p.Timestamp |y;
		p.Timestamp = p.Timestamp >> 8;
		//printf("Décompo y = %u\n",y );
		x=p.Timestamp |x;
		p.Timestamp = p.Timestamp >> 8;
		//printf("Décompo x = %u\n",x);
		w=p.Timestamp |w;
		//printf("Décompo w = %u\n",w );




		//encodage du Timestamp
		*((*b).content+(i+1))  = *((*b).content+(i+1))  | w;
		*((*b).content+(i+2))  = *((*b).content+(i+2))  | x;
		*((*b).content+(i+3))  = *((*b).content+(i+3))  | y;
		*((*b).content+(i+4))  = *((*b).content+(i+4))  | z;
	


	

	
}

void buffToStruct(Paquet *p, Buffer b){
	//init à 0
	(*p).type=0;
	(*p).TR=0;
	(*p).window=0;
	(*p).L=0;
	(*p).length7=0;
	(*p).length15=0;
	(*p).Seqnum=0;
	(*p).Timestamp=0;

	//copy de b.c et shift le bit L au bord(droit) de copy 
	uint32_t copy=*(b.content+1);
	copy = copy >> 7;
	//printf("copy = %u\n",copy);
	

	//coeur de methode

	(*p).window = (*p).window | *(b.content);
	*(b.content) = *(b.content) >> 5;
	(*p).TR = (*p).TR | *(b.content);
	*(b.content) = *(b.content) >> 1;
	(*p).type = (*p).type | *(b.content);

	//i => indice de décalage
	int i;

	//si  L==0
	if(copy == 0){
		(*p).L = 0;
		(*p).length7 = (*p).length7 | *(b.content+1);
		i=2;
	}
	//si L!=0
	else if(copy == 1){
		i=3;
		(*p).L = 1;
		//recomposition length15
		uint8_t l1 = 0;
		uint8_t l2 = 0;
		l1 = *(b.content+1);
		l2 = *(b.content+2);
		//printf("l1 = %u\n",l1);
		//printf("l2 = %u\n",l2);
		(*p).length15 = (*p).length15 | l1;
		//printf("%u\n", (*p).length15);
		(*p).length15 = (*p).length15 <<8;
		(*p).length15 = (*p).length15 | l2;


	}
	else {printf("bug lors du décodage (L!=0 et L!=1)\n");}
	
	(*p).Seqnum = (*p).Seqnum | *(b.content+i);

	//recomposition Timestamp
	uint8_t w =0;
	uint8_t x =0;
	uint8_t y =0;
	uint8_t z =0;

	w = w | *(b.content+(i+1));
	x = x | *(b.content+(i+2));
	y = y | *(b.content+(i+3));
	z = z | *(b.content+(i+4));

	(*p).Timestamp = (*p).Timestamp | w;
	(*p).Timestamp = (*p).Timestamp << 8;
	(*p).Timestamp = (*p).Timestamp | x;
	(*p).Timestamp = (*p).Timestamp << 8;
	(*p).Timestamp = (*p).Timestamp | y;
	(*p).Timestamp = (*p).Timestamp << 8;
	(*p).Timestamp = (*p).Timestamp | z;



}



void display(Paquet p){
	
	printf("type : %u\n",p.type );
	printf("TR : %u\n",p.TR );
	printf("window : %u\n",p.window );
	printf("L : %u\n",p.L );
	if(p.L==0){
		printf("length : %u\n",p.length7 );
	}
	else if(p.L==1){
		printf("length : %u\n",p.length15 );
	}
	
	printf("Seqnum : %u\n",p.Seqnum);
	printf("Timestamp : %u\n",p.Timestamp);

}


