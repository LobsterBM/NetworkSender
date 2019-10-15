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
void structToBuff(Paquet t, Buffer b);
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

	structToBuff(p,buff);
	//fin test d'encodage

    return 0;
}

//buffer needs to be 32-bits
void structToBuff(Paquet p, Buffer b){
	
	b.c = 0;
	//printf("step 1:%u\n",b.c );
	b.c = b.c | p.type;
	//printf("step 2:%u\n",b.c );
	b.c = b.c << 1;
	b.c = b.c | p.TR;
	//printf("step 3:%u\n",b.c );//ok jusqu'ici
	b.c = b.c << 5;
	b.c = b.c | p.window;
	//printf("step 4:%u\n",b.c );
	b.c = b.c << 1;
	b.c = b.c | p.L;
	//printf("step 5:%u\n",b.c );//ok
	b.c = b.c << 7;
	b.c = b.c | p.length7;
	b.c = b.c << 8;
	b.c = b.c << 8;
	b.c = b.c | p.Seqnum;

	
	

	printf("final step:%u\n",b.c );


	
}