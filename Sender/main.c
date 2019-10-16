#include <stdio.h>
#include <string.h>

char * filename;
char * address;
int port ;

int ArgReader(int argc, char *argv[]){
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



int main(int argc, char *argv[] ) {

    for(int i  = 1 ; i < argc ; i++){
        printf(argv[i]);
        printf("\n");
    }


    printf("end \n");
    return 0;
}