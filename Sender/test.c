#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <zlib.h>
#include "functions.h"
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

int init_suite(void) { return 0; }
int clean_suite(void) { return 0; }



void packet_creation(void){
    Paquet p = packetConstructor(1,0,1,0,5,0,3);
    if(p.type != 1) CU_FAIL("Packet does not match given values ");
    if(p.TR != 0) CU_FAIL("Packet does not match given values ");
    if(p.window != 1) CU_FAIL("Packet does not match given values ");
    if(p.L != 0) CU_FAIL("Packet does not match given values ");
    if(p.length7 != 5) CU_FAIL("Packet does not match given values ");
    if(p.Seqnum != 0) CU_FAIL("Packet does not match given values ");
    if(p.Timestamp != 3) CU_FAIL("Packet does not match given values ");

    CU_ASSERT(1);


}

void packet_buffer_test(void){

    struct Paquet p = packetConstructor(1,0,1,0,5,0,3);
    char * buffertest = malloc(sizeof(char)*(15) + sizeof("test text"));
    if(buffertest == NULL){
        CU_FAIL("Pointer is NULL");
    }
    CU_ASSERT(1);
}


int main(){
    CU_pSuite pSuite1 = NULL;

    if(CUE_SUCCESS != CU_initialize_registry()){
        return CU_get_error();
    }

    pSuite1 = CU_add_suite("sender_testing", init_suite, clean_suite);
    if (NULL == pSuite1) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if ((NULL == CU_add_test(pSuite1, "packet initialisation", packet_creation)) ||
        (NULL == CU_add_test(pSuite1, "packet assignment to buffer", packet_buffer_test))
        )
    {
        CU_cleanup_registry();
        return CU_get_error();
    }


    CU_basic_run_tests();
  //  CU_basic_show_failures(CU_get_failure_list());
  return CU_get_error();

}



