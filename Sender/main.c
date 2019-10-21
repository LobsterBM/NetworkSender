#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <zlib.h>




char * filename = NULL;
char * address;
int port ;
FILE *file;
uint32_t crc ;

int argReader(int argc, char *argv[]){
    //TODO argument arror check
    for(int i = 1 ; i < argc ; i++){
        if(strstr(argv[i],"-f" ) != NULL){
            i++;
            filename = argv[i];
            //TODO extract filename from position i+1
        }
        else{
            address = argv[i];
            i++; //port numer
            port = atoi(argv[i]);
            i = argc;
            i++;
        }
    }
    if (filename == NULL){
        file = stdin;
    }
    else{
        file = file = fopen(filename , "rb");
        //TODO errror check
    }


}

int getPayload(char * payload, FILE * file, int byteRead){

    memset(payload, 0, sizeof(char)*byteRead);

   ssize_t bytesRead = fread(payload, sizeof(char), byteRead, file);
   if (bytesRead == 0){
       //TODO error
   }
   printf("bytes read : %d \n" , (int)bytesRead);
   printf(payload);
   return bytesRead;

}





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




//fonctions définies
void display(Paquet p);
void structToBuff(Paquet p, Buffer *b);
void buffToStruct(Paquet *p, Buffer b);
//fin


char * packetGenerator ( Paquet p , char * payload , int payloadLen){
    //TODO TR 1 case
    // shift in case of length 7 instead of 15
    int shift = 0;
    if(!(p.L)) shift = 1;

	char *packet = malloc(sizeof(char)*(12-shift+payloadLen));

    char* header[8-shift];

    Buffer headBuff ;
    headBuff.content = calloc(8-shift,sizeof(char));
    if(headBuff.content==NULL){printf("Le malloc a échoué\n");}
    structToBuff(p,&headBuff);
    //packet to buffer


    //header buffer to final packet buffer
    for(int i = 0 ; i < 8-shift ; i++){
        header[i] =headBuff.content[i];
    }

    //copy header to final packet
    for(int i = 0 ; i < 8-shift ; i++){
        packet[i] = header[i];
    }

/*
    for(int i = 0 ; i < 64*64 ; i++){
        crc=0;
        crc  = (uint32_t) crc32(crc , (Bytef *)(headBuff.content), i);
        if(crc == 568805601){
            printf("crc value found non htonl %d \n", i);
        }
        if(htonl(crc) == 568805601){
            printf("crc value found htonl %d", i);
        }

    }
*/

    crc= 0;
    crc  = (uint32_t) crc32(crc , (Bytef *)(headBuff.content), 8);
    //crc= 2214560385;
    uLong crccpy=crc;
    printf("crc 1 : %lu \n" , crc);
    printf("crc 2 : %lu \n" , htonl(crc));
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


    return packet;



}

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


int main (int argc, char **argv){

    printf("Starting ... \n");

  	crc = crc32(0L,Z_NULL,0);
	//crc needs to be initialised at each startup

	argReader(argc,argv);

	printf("Args processed \n");
    printf("address %s , port %d  \n" , address , port);

    char * payload[512] = {0};

    int payLen = getPayload(payload, file, 512);


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

	Paquet p = packetConstructor(2,0,0,1,payLen,1,1);


	printf("\n -> %d", payLen);
	printf("\n -> %lu", htons(payLen));
	printf("\n -> %lu", htonl(payLen));
	printf("\n -> %lu", p.length15);

	char * finalbuffer = malloc(sizeof(char)*(payLen+12-p.L));
//	finalbuffer = packetGenerator(p,payload,payLen);

	memcpy(finalbuffer,packetGenerator(p,payload,payLen),sizeof(char)*(payLen+12-p.L));

    char tmp = 'c';
    for (int i = 0 ; i < payLen+12 ; i++){
        tmp=finalbuffer[i];
    }



    sent = sendto(sock,finalbuffer,(payLen+17-p.L)*sizeof(char),0,(const struct sockaddr *)&peer_addr, sizeof(peer_addr));



    printf("\n payload len : %d \n" , payLen);





	return 0;}

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


