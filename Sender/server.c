#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>


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
//fin


int main (int argc, char **argv){

	//init part
	int sock = socket(AF_INET6,SOCK_DGRAM,0);
	if(sock == -1 ){printf("Erreur lors de la création des sockets.\n");}

	struct sockaddr_in6 peer_addr;                      // allocate the address on the stack
	memset(&peer_addr, 0, sizeof(peer_addr));           // fill the address with 0-bytes to avoid garbage-values
	peer_addr.sin6_family = AF_INET6;                   // indicate that the address is an IPv6 address
	peer_addr.sin6_port = htons(55555);                 // indicate that the programm is running on port 55555
	inet_pton(AF_INET6, "::1", &peer_addr.sin6_addr);   // indicate that the program is running on the computer identified by the ::1 IPv6 address

	//receving part
	ssize_t buff_maxsize = 512;
	
	socklen_t peer_addr_len = sizeof(peer_addr);

	ssize_t bound = bind(sock,(const struct sockaddr*)&peer_addr,peer_addr_len);
	if(bound == -1){printf("Erreur lors de la liaison\n");}
	



	while(1){
		printf("Listenning..\n");
		Buffer buff;
		buff.content = calloc(32,sizeof(char));
		if(buff.content==NULL){printf("Le malloc a échoué\n");}
		Paquet paquetRecu;
		
		
		
		ssize_t reception = recv(sock,buff.content,32*sizeof(char),0); //réception du buffer
		
		//ssize_t reception = recvfrom(sock2,buff,buff_maxsize,0,(struct sockaddr*)&peer_addr,&peer_addr_len);
		if(reception == -1){printf("Erreur lors de la réception des données.\n");}

		//printf("reception length :%d\n", (int)reception);
		for(int i=0;i<32;i++){
			printf("%d: %d\n",i, *(buff.content+i));
		}

		buffToStruct(&paquetRecu,buff);
		display(paquetRecu);
		
		printf("\n");
		printf("\n");

		for(int i=0;i<32;i++){
			printf("%d: %d\n",i, *(buff.content+i));
		}
		
		

	}

	/* ancien
	//bind_and_receive_from_peer(sock,(struct sockaddr*)&peer_addr,peer_addr_len);
	while(1){
		printf("Listenning..\n");
		char buff[buff_maxsize];
		

		init_buff(buff,buff_maxsize);
		
		ssize_t reception = recv(sock,buff,(size_t)buff_maxsize,0); // réception string
		
		//ssize_t reception = recvfrom(sock2,buff,buff_maxsize,0,(struct sockaddr*)&peer_addr,&peer_addr_len);
		if(reception == -1){printf("Erreur lors de la réception des données.\n");}

		
		printf("%s\n",buff);
		
		

	}*/
	

	printf("Server closed.\n");


return 0;}

int init_buff(char *buff, int buff_len){
	for(int i=0;i<buff_len;i++){
		buff[i]=0;
	}

	return 0;
}



void structToBuff(Paquet p, Buffer *b){
	
	*((*b).content)  = 0;
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
	*((*b).content+1)  = *((*b).content+1) | p.L;
	//printf("byte 2 setp 2:%u\n",*((*b).content+1));
	
	if(p.L==0){
		*((*b).content+1)  = *((*b).content+1) << 7;
		*((*b).content+1)  = *((*b).content+1) | p.length7;
		//printf("byte 2 setp 3:%u\n",*((*b).content+1));
	}
	else{
		//décomposition length15
		uint8_t l1 =0;
		uint8_t l2 =0;
		l2 =  p.length15 | l2;
		p.length15 = p.length15 >> 8;
		l1= p.length15 | l1;
		//printf("l1: %u\n",l1);
		//printf("l2: %u\n",l2);

		//encodage du length15
		*((*b).content+1)  = *((*b).content+1)+128 | l1;
		*((*b).content+2)  = *((*b).content+2) | l2;
	}

	
	*((*b).content+3)  = *((*b).content+3)  | p.Seqnum;
	//printf("byte 4 setp 1:%u\n",*((*b).content+3));

	
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
	*((*b).content+4)  = *((*b).content+4)  | w;
	*((*b).content+5)  = *((*b).content+5)  | x;
	*((*b).content+6)  = *((*b).content+6)  | y;
	*((*b).content+7)  = *((*b).content+7)  | z;
	


	

	
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
	printf("buff1:%u\n",*(b.content) );
	*(b.content) = *(b.content) >> 5;
	(*p).TR = (*p).TR | *(b.content);
	*(b.content) = *(b.content) >> 1;
	(*p).type = (*p).type | *(b.content);



	//si  L==0
	if(copy == 0){
		(*p).L = 0;
		(*p).length7 = (*p).length7 | *(b.content+1);
	}
	//si L!=0
	else if(copy == 1){
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
	
	(*p).Seqnum = (*p).Seqnum | *(b.content+3);

	//recomposition Timestamp
	uint8_t w =0;
	uint8_t x =0;
	uint8_t y =0;
	uint8_t z =0;

	w = w | *(b.content+4);
	x = x | *(b.content+5);
	y = y | *(b.content+6);
	z = z | *(b.content+7);

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

