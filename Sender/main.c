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
//fin




int main (int argc, char **argv){


Buffer buff;
buff.content = calloc(32,sizeof(char));
if(buff.content==NULL){printf("Le malloc a échoué\n");}

//printf("Size of content: %d\n", (int)sizeof(*(buff.content)));

Paquet p,p2;
p.type = 2;
p.TR = 0;
p.window = 10;
p.L = 0;
p.length7 = 46;
p.length15 = 29018;
p.Seqnum = 198;
p.Timestamp = 188632383;




structToBuff(p,&buff);
buffToStruct(&p2,buff);
display(p2);






return 0;}


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
	*(b.content) = *(b.content) >> 5;
	(*p).TR = (*p).TR | *(b.content);
	*(b.content) = *(b.content) >> 1;
	(*p).type = (*p).type | *(b.content);



	//si  L==0
	if(copy == 0){
		(*p).L = 0;
		(*p).length7 = (*p).length7 | *(b.content+1);
		printf("yolo\n");
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


