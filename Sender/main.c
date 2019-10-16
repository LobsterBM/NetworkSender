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
	long *content;
};




//fonctions définies
void structToBuff(Paquet p, Buffer *b);
void buffToStruct(Paquet *p, Buffer b);
void display(Paquet p);
//fin



int main (int argc, char **argv){


Buffer buff;
buff.content = malloc(150);
if(buff.content==NULL){printf("Le malloc a échoué\n");}

//printf("Size of content: %d\n", (int)sizeof(*(buff.content)));

Paquet p,p2;
p.type = 2;
p.TR = 0;
p.window = 10;
p.L = 0;
p.length7 = 46;
p.Seqnum = 198;
p.Timestamp = 188632383;




structToBuff(p,&buff);
buffToStruct(&p2,buff);
display(p2);





return 0;}

void structToBuff(Paquet p, Buffer *b){
	
	*((*b).content)  = 0;
	

	//printf("step 1:%u\n",b.c );
	*((*b).content)= *((*b).content) | p.type;
	//printf("step 2:%u\n",b.c );
	*((*b).content)= *((*b).content) << 1;
	*((*b).content)= *((*b).content) | p.TR;
	//printf("step 3:%u\n",b.c );//ok jusqu'ici
	*((*b).content) = *((*b).content)  << 5;
	*((*b).content)  = *((*b).content)  | p.window;
	//printf("step 4:%u\n",b.c );
	*((*b).content)  = *((*b).content)  << 1;
	*((*b).content)  = *((*b).content) | p.L;
	//printf("step 5:%u\n",b.c );//ok
	if(p.L==0){
		*((*b).content)  = *((*b).content) << 7;
		*((*b).content)  = *((*b).content) | p.length7;
		*((*b).content)  = *((*b).content) << 8;
	}
	else{
		*((*b).content) = *((*b).content)  << 15;
		*((*b).content)  = *((*b).content)  | p.length15;
	}
	*((*b).content)  = *((*b).content) << 8;
	*((*b).content)  = *((*b).content)  | p.Seqnum;

	/*
	*((*b).content)  = *((*b).content) << 32;
	*((*b).content)  = *((*b).content)  | p.Timestamp;
	*/


	

	//printf("final step:%u\n",*((*b).content));


	
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
	uint32_t copy=*(b.content) ;
	copy= copy >> 23;
	

	//coeur de methode
	/*	
	(*p).Timestamp = (*p).Timestamp | *(b.content);
	*(b.content) =*(b.content)>>32;
	*/

	(*p).Seqnum = (*p).Seqnum | *(b.content);
	*(b.content) =*(b.content)>>8;

	//si  L==0
	if(copy%2 == 0){
		(*p).L = 0;
		*(b.content) =*(b.content) >>8;
		(*p).length7 = (*p).length7 | *(b.content);
		*(b.content) =*(b.content)>>8;
	}
	//si L!=0
	else {
		(*p).L = 1;
		(*p).length15 = (*p).length15 | *(b.content);
		*(b.content) =*(b.content) >>16;
	}
	
	(*p).window = (*p).window | *(b.content);
	*(b.content)=*(b.content) >>5;

	(*p).TR = (*p).TR |*(b.content);
	*(b.content) =*(b.content) >>1;

	(*p).type = (*p).type | *(b.content);
	




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