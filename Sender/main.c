#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>



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
	unsigned int d : 32;
};


//fonctions définies
void structToBuff(Paquet t, Buffer *b);
void buffToStruct(Paquet *p, Buffer b);
void display(Paquet p);
//fin



int main(int argc, char *argv[] ) {

   
   //début test d'encodage 
    Buffer buff;
	Paquet p;
	p.type = 2;
	p.TR = 0;
	p.window = 10;
	p.L = 0;
	p.length7 = 46;
	p.Seqnum = 198;

	structToBuff(p,&buff);
	

	//décodage
	Paquet p2;
	buffToStruct(&p2,buff);
	display(p2);


    return 0;
}

//buffer needs to be 32-bits
void structToBuff(Paquet p, Buffer *b){
	
	(*b).c = 0;
	

	//printf("step 1:%u\n",b.c );
	(*b).c = (*b).c | p.type;
	//printf("step 2:%u\n",b.c );
	(*b).c = (*b).c << 1;
	(*b).c = (*b).c | p.TR;
	//printf("step 3:%u\n",b.c );//ok jusqu'ici
	(*b).c = (*b).c << 5;
	(*b).c = (*b).c | p.window;
	//printf("step 4:%u\n",b.c );
	(*b).c = (*b).c << 1;
	(*b).c = (*b).c | p.L;
	//printf("step 5:%u\n",b.c );//ok
	if(p.L==0){
		(*b).c = (*b).c << 7;
		(*b).c = (*b).c | p.length7;
		(*b).c = (*b).c << 8;
	}
	else{
		(*b).c = (*b).c << 15;
		(*b).c = (*b).c | p.length15;
	}
	(*b).c = (*b).c << 8;
	(*b).c = (*b).c | p.Seqnum;


	

	//printf("final step:%u\n",(*b).c );


	
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

	//copy de b.c et shift le bit L au bord(droit) de copy 
	uint32_t copy=b.c;
	copy= copy >> 23;
	

	//coeur de methode
	(*p).Seqnum = (*p).Seqnum | b.c;
	b.c=b.c>>8;

	//si  L==0
	if(copy%2 == 0){
		(*p).L = 0;
		b.c=b.c>>8;
		(*p).length7 = (*p).length7 | b.c;
		b.c=b.c>>8;
	}
	//si L!=0
	else {
		(*p).L = 1;
		(*p).length15 = (*p).length15 | b.c;
		b.c=b.c>>16;
	}
	
	(*p).window = (*p).window | b.c;
	b.c=b.c>>5;

	(*p).TR = (*p).TR | b.c;
	b.c=b.c>>1;

	(*p).type = (*p).type | b.c;
	




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

}
