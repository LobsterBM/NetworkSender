#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/time.h>


//structures
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
	unsigned char *networkContent;
};
//fin structures


//fonctions définies
void display(Paquet p);
void structToBuff(Paquet p, Buffer *b);
void buffToStruct(Paquet *p, Buffer b);
int is_time_out(struct timeval t, int timeout);
//fin



int main (int argc, char **argv){

	//init part
	int sock = socket(AF_INET6,SOCK_DGRAM,0);
	if(sock == -1 ){printf("Erreur lors de la création des sockets.\n");}

	struct sockaddr_in6 peer_addr;                      // allocate the address on the stack
	memset(&peer_addr, 0, sizeof(peer_addr));           // fill the address with 0-bytes to avoid garbage-values
	peer_addr.sin6_family = AF_INET6;
	if(argc>1){
		int portDest = atoi(argv[1]);
		peer_addr.sin6_port = htons(portDest);  
	}           
	else{
		peer_addr.sin6_port = htons(55555);  
	}        // indicate that the address is an IPv6 address
	               // indicate that the programm is running on port 55555
	inet_pton(AF_INET6, "::1", &peer_addr.sin6_addr);   // indicate that the program is running on the computer identified by the ::1 IPv6 address


	//poll stuff
	int fds_length =2;
	struct pollfd fds[fds_length];
	int timeout = 3000;
	ssize_t sent=0;

	//reception stuff
	int buff_maxsize =32;
	char *buff= calloc(buff_maxsize,sizeof(char));
	if(buff==NULL){printf("fail de malloc\n");}
	char *buffSend= calloc(buff_maxsize,sizeof(char));
	if(buffSend==NULL){printf("fail de malloc\n");}

	struct sockaddr_in6 peer2_addr;
	socklen_t peer2_len = sizeof(peer2_addr);

		int count =0;
		int window=2;
		int seqnumLength =4;
		int seqnum[4] = {3,4,5,6};
		int sentTab[4]= {0,0,0,0};// 0: non envoyé, 1: envoyé mais pas ack, 2: envoyé et ack

		
		int a=0;

	//boucle principale 
	while(count<seqnumLength){
			fds[0].fd=sock;
			fds[0].events = 0;
			fds[0].events |= POLLIN;


			fds[1].fd=sock;
			fds[1].events = 0;
			fds[1].events |= POLLOUT;
		

			
			int pret = poll(fds,fds_length,timeout);

			
			

			if(pret>0){
				

			
				for(int i=0;i<fds_length;i++){
					if(fds[i].revents==POLLOUT && window>0 && count<seqnumLength){
						printf("POLLOUT, i: %d,  count:%d, window:%d\n",i,count,window);
						int num=-1;
						for(int j=0;j<seqnumLength;j++){
						
							if(sentTab[j]==0){
								num=seqnum[j];
								sentTab[j]=1;//envoyé
								break;
							}
						}
						//vérifie qu'il y ai bien un seqnum à envoyer
						if(num!=-1){
							sprintf(buffSend, "%d", num);
							printf("numseq envoyé: %s\n", buffSend);
							sent = sendto(sock,buffSend,sizeof(buffSend),0,(const struct sockaddr *)&peer_addr, sizeof(peer_addr));
							if(sent == -1){printf("Erreur lors de l'envoi.\n");}
							window--;
						}
						
					}
					else if(fds[i].events==1 && fds[i].revents==POLLIN){
						printf("POLLIN ");
						ssize_t reception = recvfrom(sock,buff,(size_t)buff_maxsize,0,(struct sockaddr *)&peer2_addr,&peer2_len); 
						int num = atoi(buff);
						printf("                 ack reçu:%d\n",num);
						for(int j=0;j<seqnumLength;j++){
							if(seqnum[j]==num){
								sentTab[j]=2;//ack
								//printf("test:%d\n", seqnum[j]);
								count++;
								window++;
								break;
							}
						}
						

					}
				}

				
				
			}
			


		}
	
		

	
	
	

	/*truc de base
	while(1){
		struct sockaddr_in6 peer2_addr;
		socklen_t peer2_len = sizeof(peer2_addr);
		ssize_t reception = recvfrom(sock,buff,(size_t)buff_maxsize,0,(struct sockaddr *)&peer2_addr,&peer2_len); // réception string
		printf("reception port :%d\n", peer2_addr.sin6_port);
	}*/
	printf("Client closed.\n");


return 0;}


int is_time_out(struct timeval t, int timeout){
  struct timeval stop;
  gettimeofday(&stop,NULL);
  int a = stop.tv_usec - t.tv_usec;
  printf("timelapse:%d",a);
    return 0;
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
