#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/time.h>
#include <zlib.h>




char * filename = NULL;
char * address;
int port ;
FILE *file;
uint32_t crc ;
uint32_t crc2 ;

int argReader(int argc, char *argv[]){
    //TODO argument arror check
    if(argc < 4){

        address = argv[1];
        port = atoi(argv[2]);
    }
    else if(strstr(argv[1],"-f") != NULL){
        filename = argv[2];
        address = argv[3];
        port = atoi(argv[4]);

    }
    else if (strstr(argv[3], "-f") != NULL){
        filename = argv[4];
        address = argv[1];
        port = atoi(argv[2]);

    }
    else{
        address = argv[1];
        port = atoi(argv[2]);
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

    memset(payload, 0, sizeof(char)*(byteRead));

   ssize_t bytesRead = fread(payload, sizeof(char), byteRead, file);
   if (bytesRead == 0){
       //TODO error
       return 0;
   }

   printf("bytes read : %d \n" , (int)bytesRead);

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
int is_time_out(struct timeval t, int timeout);
//fin


char * packetGenerator ( Paquet p , char * payload , int payloadLen){
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



    //init part
    int sock = socket(AF_INET6,SOCK_DGRAM,0);
    if(sock == -1 ){printf("Erreur lors de la création des sockets.\n");}

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
    char *sendingBuffer[window];
    char *receivBuffer[528];
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
				    if(payLen >= 512){
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
				    	finalbuffer = packetGenerator(p,payload,payLen);
						memcpy(finalbuffer,packetGenerator(p,(char*)payload,payLen),sizeof(char)*(payLen+16-shift));
				    }
				    
				  	

				    for(int i=0;i<windowSlide;i++){
				    	//check si timeout
				    	if(seqnumtab[i]!=-1 && is_time_out(timeSending[i],timeoutPerso)){
				    		printf("timeout dépassé i:%d\n",i);
				    		gettimeofday(&timeSending[i],NULL);
				    		sent = sendto(sock,sendingBuffer[i],sizeof(*sendingBuffer[i]),0,(const struct sockaddr *)&peer_addr, sizeof(peer_addr));
				    		}
				    	
				    	//check
				    	if(seqnumtab[i]==-1 && found==0 && payLen>0){
				    		seqnumtab[i]=num;
				    		sendingBuffer[i]=finalbuffer;
				    		gettimeofday(&timeSending[i],NULL);
				    		printf("envoyé seqnum:%d,  i:%d\n",num,i);
				    		found=1;
				    		nfull++;//on rempli de un le seqnumtab
				    		}
				    	}
				    
				    if(payLen>0){
				    	//printf("envoyé seqnum:%d",num);
				    	sent = sendto(sock,finalbuffer,payLen+16-shift,0,(const struct sockaddr *)&peer_addr, sizeof(peer_addr));
				    }	
				    


				    *payload =0;
				    payLen = getPayload((char*)payload, file, 512);
				    window--;
					}
				
				else if(fds[0].events==1 && fds[0].revents==POLLIN){
					
					ssize_t reception = recvfrom(sock,receivBuffer,sizeof(receivBuffer),0,(struct sockaddr *)&peer2_addr,&peer2_len);
					int seqnumReceiv = atoi((const char *)receivBuffer);
					printf("ack %d\n", seqnumReceiv);
					 for(int i=0;i<windowSlide;i++){
					 	//printf("vérif seqnumtab:%d et seqnumReceiv:%d\n",seqnumtab[i],seqnumReceiv );
				    	if(seqnumtab[i]==seqnumReceiv){
				    		printf("libération du buffer pour le seqnum:%d\n",seqnumReceiv);
				    		seqnumtab[i]=-1;
				    		sendingBuffer[i]="\0";
				    		window++;
				    		nfull--;//on libère le seqnum
				    		break;
				    	}
				    }
				}
			

	
		}

	    


    }

    




    //cloture
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



 int is_time_out(struct timeval t, int timeout){
  struct timeval stop;
  gettimeofday(&stop,NULL);
  time_t b = stop.tv_sec -t.tv_sec;
  b*= 1000000;
  time_t a = stop.tv_usec - t.tv_usec;
  a+=b;
 // printf("timelapse:%ld\n",a);
  if(a>timeout){
  	printf("timelapse:%ld\n",a);
  	return 1;}
    return 0;
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


