#include <stdio.h>
#include <string.h>

char * filename;
char * address;
int port ;

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
            port = argv[i];
        }
    }

}

int getPayload(char * payload, FILE * file){

    memset(payload, 0, sizeof(char)*64);

   ssize_t bytesRead = fread(payload, sizeof(char), 64 , file);
   if (bytesRead == 0){
       //TODO error
   }
   printf("bytes read : %d \n" , (int)bytesRead);
   printf(payload);

}



int main(int argc, char *argv[] ) {

    for(int i  = 1 ; i < argc ; i++){
        printf(argv[i]);
        printf("\n");
    }


    printf("end \n");

    argReader(argc,argv);

    FILE *file;
    printf(filename);

    file = fopen(filename , "rb");


    if(file == NULL){
        printf("Error , can't open file \n");
        return -1;
    }

    char * payload[64] = {0};

    for(int i = 0 ; i < 100 ; i++){
        printf("\n");

        getPayload(payload,file);

    }



    printf("\n");
    return 0;
}