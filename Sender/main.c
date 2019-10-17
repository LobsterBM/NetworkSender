#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char * filename = NULL;
char * address;
int port ;
FILE *file;

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

}



int main(int argc, char *argv[] ) {




    for(int i  = 1 ; i < argc ; i++){
        printf(argv[i]);
        printf("\n");
    }


    printf("end \n");

    argReader(argc,argv);



    if(file == NULL){
        printf("Error , can't open file \n");
        return -1;
    }

    char * payload[512] = {0};

    for(int i = 0 ; i < 12 ; i++){
        printf("\n\n\n\n");

        getPayload(payload,file, 512);

    }




    printf("\n");
    printf("address is %s : %d \n", address ,port);
    return 0;
}