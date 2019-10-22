
#include <stdio.h>
#include <stdlib.h>
#include "main.c"
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

int init_suite(void) { return 0; }
int clean_suite(void) { return 0; }


/*
 * Tests if initialised matrix is != NULL
 */
void naive_matrix_creation(void){
    struct matrix *test = matrix_init(5,5);
    CU_ASSERT_PTR_NOT_NULL(test);
}


/*
 * Fills up matrix in order
 */
void naive_matrix_sequential_assignment(void){
    struct matrix *test = matrix_init(5,5);

    for(int i = 0 ; i < (int)test->nlines ; i++){
        for(int j = 0 ; j < (int)test->ncols ; j++){
            matrix_set(test,i,j,(test->ncols*i)+j+1);
            if(matrix_get(test,i,j)  != ((test->ncols*i)+j+1)){
                CU_FAIL("Sequential naive assignment did not match given values");
            }
        }
    }

    CU_ASSERT(1); // tests passed
}


/*
 * fills up matrix in reverse order
 */
void naive_matrix_random_backwards_assignment(void){
    struct matrix *test = matrix_init(5,5);
    double randVal;

    for(int i = test->nlines-1 ; i >= 0  ; i--){
        for(int j = test->ncols-1 ; j >= 0  ; j--){
            randVal = rand()/2.0;
            matrix_set(test,i,j,randVal);
            if(matrix_get(test,i,j)  != randVal){
                CU_FAIL("Random naive  assignment did not match given values");
            }
        }
    }

    CU_ASSERT(1); // tests passed
}


/*
 * Transforms a naive matrix to a sparse one
 */
void naive_to_sparse(void){

    struct matrix *test = matrix_init(5,5);

    for(int i = 0 ; i < (int)test->nlines ; i++){
        for(int j = 0 ; j < (int)test->ncols ; j++){
            matrix_set(test,i,j,(test->ncols*i)+j+1);
        }
    }

    struct sp_matrix* sptest = matrix_to_sp_matrix(test , 0.00001);

    for(int i = 0 ; i < (int)test->nlines ; i++){
        for(int j = 0 ; j < (int)test->ncols ; j++){
            if(matrix_get(test,i,j)  != sp_matrix_get(sptest,i,j)){
                CU_FAIL("Naive to sparse values do not match.");
            }
        }
    }

    CU_ASSERT(1);
}


/*
 * Transpose matrix
 */
void naive_sequential_transpose(void){

    struct matrix *test = matrix_init(5,5);

    for(int i = 0 ; i < (int)test->nlines ; i++){
        for(int j = 0 ; j < (int)test->ncols ; j++){
            matrix_set(test,i,j,(test->ncols*i)+j+1);
        }
    }

    struct matrix * transposed = matrix_transpose(test);

    for(int i = 0 ; i < (int)test->nlines ; i++){
        for(int j = 0 ; j < (int)test->ncols ; j++){

            if(matrix_get(test,i,j)  != matrix_get(transposed,j,i)){
                CU_FAIL("Transposed naive matrix did not match given values");
            }
        }
    }

    CU_ASSERT(1); // tests passed
}


/*
 * Adds a matrix to itself and checks each cell
 */
void naive_addition(void){
    struct matrix *test = matrix_init(3,3);

    for(int i = 0 ; i < (int)test->nlines ; i++){
        for(int j = 0 ; j < (int)test->ncols ; j++){
            matrix_set(test,i,j,(test->ncols*i)+j+1);
        }
    }

    struct matrix * summed = matrix_add(test,test);
    struct matrix * correct = matrix_init(3,3);
    matrix_set(correct,0,0,2);
    matrix_set(correct,0,1,4);
    matrix_set(correct,0,2,6);
    matrix_set(correct,1,0,8);
    matrix_set(correct,1,1,10);
    matrix_set(correct,1,2,12);
    matrix_set(correct,2,0,14);
    matrix_set(correct,2,1,16);
    matrix_set(correct,2,2,18);

    for(int i = 0 ; i < (int)test->nlines ; i++){
        for(int j = 0 ; j < (int)test->ncols ; j++){

            if(matrix_get(summed,i,j)  !=  matrix_get(correct,i,j)){
                CU_FAIL("Naive addition did not match given values");
            }
        }
    }

    CU_ASSERT(1);
}


/*
 * multiplies a matrix to itself and checks each cell with a control matrix
 */

void naive_multiplication(void){
    struct matrix *test = matrix_init(3,3);

    for(int i = 0 ; i < (int)test->nlines ; i++){
        for(int j = 0 ; j < (int)test->ncols ; j++){
            matrix_set(test,i,j,(test->ncols*i)+j+1);
        }
    }

    struct matrix * mult = matrix_mult(test,test);
    struct matrix * correct = matrix_init(3,3);
    matrix_set(correct,0,0,30);
    matrix_set(correct,0,1,36);
    matrix_set(correct,0,2,42);
    matrix_set(correct,1,0,66);
    matrix_set(correct,1,1,81);
    matrix_set(correct,1,2,96);
    matrix_set(correct,2,0,102);
    matrix_set(correct,2,1,126);
    matrix_set(correct,2,2,150);

    for(int i = 0 ; i < (int)test->nlines ; i++){
        for(int j = 0 ; j < (int)test->ncols ; j++){

            if(matrix_get(mult,i,j)  !=  matrix_get(correct,i,j)){
                CU_FAIL("Naive multiplication did not match given values");
            }
        }
    }

    CU_ASSERT(1);
}


/*
 * checks if file saves and load correctly
 */
void naive_save_load(void){

    struct matrix *test = matrix_init(3,3);

    for(int i = 0 ; i < (int)test->nlines ; i++){
        for(int j = 0 ; j < (int)test->ncols ; j++){
            matrix_set(test,i,j,(test->ncols*i)+j+1);
        }
    }

    matrix_save(test, "naiveUnitLoadTest");
    struct matrix* loaded = matrix_load("naiveUnitLoadTest");
    if(!loaded){
        CU_FAIL("Matrix was not loaded from file");
    }
    remove("naiveUnitLoadTest");

    for(int i = 0 ; i < (int)test->nlines ; i++){
        for(int j = 0 ; j < (int)test->ncols ; j++){

            if(matrix_get(test,i,j)  !=  matrix_get(loaded,i,j)){
                CU_FAIL("Loaded naive matrix from file did not match given values");
            }
        }
    }

    CU_ASSERT(1);
}


/*
 * Tests if initialised matrix is != NULL
 */
void sparse_matrix_creation(void){

    struct sp_matrix *test = sp_matrix_init(0.0001 ,5,5);
    CU_ASSERT_PTR_NOT_NULL(test);
}


/*
 * Fills up matrix in order
 */
void sparse_matrix_sequential_assignment(void){

    struct sp_matrix *test = sp_matrix_init(0.0001,5,5);

    for(int i = 0 ; i < (int)test->nlines ; i++){
        for(int j = 0 ; j < (int)test->ncols ; j++){
            sp_matrix_set(test,i,j,(test->ncols*i)+j+1);
            if(sp_matrix_get(test,i,j)  != ((test->ncols*i)+j+1)){
                CU_FAIL("Sequential sparse assignment did not match given values");
            }
        }
    }

    CU_ASSERT(1); // tests passed
}


/*
 * fills up matrix in reverse order
 */
void sparse_matrix_random_backwards_assignment(void){
    struct sp_matrix *test = sp_matrix_init(0.0001,5,5);
    double randVal;

    for(int i = test->nlines-1 ; i >= 0  ; i--){
        for(int j = test->ncols ; j >= 0  ; j--){
            randVal = rand()/2.0;
            sp_matrix_set(test,i,j,randVal);
            if(sp_matrix_get(test,i,j)  != randVal){
                CU_FAIL("Random sparse assignment did not match given values");
            }
        }
    }

    CU_ASSERT(1); // tests passed
}


/*
 * fills up a sparse matrix diagonally and checks each cell
 */
void sparse_assignment_diagonal(void){

    struct sp_matrix *test = sp_matrix_init(0.0001,5,5);

    for(int i = 0 ; i < (int)test->nlines ; i++){
        sp_matrix_set(test,i,i,i);
    }

    for(int i = 0 ; i < (int)test->nlines ; i++){
        if(sp_matrix_get(test,i,i) != (double) i ){
            CU_FAIL("Diagonal sparse assignment did not match control values");
        }
    }

    CU_ASSERT(1); // tests passed
}


/*
 * Transforms a sparse matrix to a naive one
 */
void sparse_to_naive(void){

    struct sp_matrix *test = sp_matrix_init(0.0001,5,5);

    for(int i = 0 ; i < (int)test->nlines ; i++){
        for(int j = 0 ; j < (int)test->ncols ; j++){
            sp_matrix_set(test,i,j,(test->ncols*i)+j+1);
        }
    }

    struct matrix* naivetest = sp_matrix_to_matrix(test);

    for(int i = 0 ; i < (int)test->nlines ; i++){
        for(int j = 0 ; j < (int)test->ncols ; j++){
            if(sp_matrix_get(test,i,j)  != matrix_get(naivetest,i,j)){
                CU_FAIL("Sparse to naive values do not match.");
            }
        }
    }

    CU_ASSERT(1);
}


/*
 * Transpose matrix
 */
void sparse_sequential_transpose(void){

    struct sp_matrix *test = sp_matrix_init(0.0001,5,5);

    for(int i = 0 ; i < (int)test->nlines ; i++){
        for(int j = 0 ; j < (int)test->ncols ; j++){
            sp_matrix_set(test,i,j,(test->ncols*i)+j+1);
        }
    }

    struct sp_matrix * transposed = sp_matrix_transpose(test);

    for(int i = 0 ; i < (int)test->nlines ; i++){
        for(int j = 0 ; j < (int)test->ncols ; j++){

            if(sp_matrix_get(test,i,j)  != sp_matrix_get(transposed,j,i)){
                CU_FAIL("Sequential sparse assignment did not match given values");
            }
        }
    }

    CU_ASSERT(1); // tests passed
}


/*
 * Adds a matrix to itself and checks each cell
 */
void sparse_addition(void){


    struct sp_matrix *test = sp_matrix_init(0.0001,3,3);

    for(int i = 0 ; i < (int)test->nlines ; i++){
        for(int j = 0 ; j < (int)test->ncols ; j++){
            sp_matrix_set(test,i,j,(test->ncols*i)+j+1);
        }
    }

    struct sp_matrix * summed = sp_matrix_add(test,test);
    struct sp_matrix * correct = sp_matrix_init(0.0001,3,3);
    sp_matrix_set(correct,0,0,2);
    sp_matrix_set(correct,0,1,4);
    sp_matrix_set(correct,0,2,6);
    sp_matrix_set(correct,1,0,8);
    sp_matrix_set(correct,1,1,10);
    sp_matrix_set(correct,1,2,12);
    sp_matrix_set(correct,2,0,14);
    sp_matrix_set(correct,2,1,16);
    sp_matrix_set(correct,2,2,18);

    for(int i = 0 ; i < (int)test->nlines ; i++){
        for(int j = 0 ; j < (int)test->ncols ; j++){

            if(sp_matrix_get(summed,i,j)  !=  sp_matrix_get(correct,i,j)){
                CU_FAIL("Sparse addition did not match given values");
            }
        }
    }

    CU_ASSERT(1);
}


/*
 * multiplies a matrix to itself and checks each cell with a control matrix
 */
void sparse_multiplication(void){


    struct sp_matrix *test = sp_matrix_init(0.0001,3,3);

    for(int i = 0 ; i < (int)test->nlines ; i++){
        for(int j = 0 ; j < (int)test->ncols ; j++){
            sp_matrix_set(test,i,j,(test->ncols*i)+j+1);
        }
    }

    struct sp_matrix * mult = sp_matrix_mult(test,test);
    struct sp_matrix * correct = sp_matrix_init(0.0001,3,3);
    sp_matrix_set(correct,0,0,30);
    sp_matrix_set(correct,0,1,36);
    sp_matrix_set(correct,0,2,42);
    sp_matrix_set(correct,1,0,66);
    sp_matrix_set(correct,1,1,81);
    sp_matrix_set(correct,1,2,96);
    sp_matrix_set(correct,2,0,102);
    sp_matrix_set(correct,2,1,126);
    sp_matrix_set(correct,2,2,150);

    for(int i = 0 ; i < (int)test->nlines ; i++){
        for(int j = 0 ; j < (int)test->ncols ; j++){

            if(sp_matrix_get(mult,i,j)  !=  sp_matrix_get(correct,i,j)){
                CU_FAIL("Sparse multiplication did not match given values");
            }
        }
    }

    CU_ASSERT(1);
}


/*
 * checks if file saves and load correctly
 */
void sparse_save_load(void){

    struct sp_matrix *test = sp_matrix_init(0.0001,3,3);

    for(int i = 0 ; i < (int)test->nlines ; i++){
        for(int j = 0 ; j < (int)test->ncols ; j++){
            sp_matrix_set(test,i,j,(test->ncols*i)+j+1);
        }
    }

    sp_matrix_save(test, "sparseUnitLoadTest");
    struct sp_matrix* loaded = sp_matrix_load("sparseUnitLoadTest");
    if(!loaded){CU_FAIL("Matrix was not loaded from file")};
    remove("sparseUnitLoadTest");

    for(int i = 0 ; i < (int)test->nlines ; i++){
        for(int j = 0 ; j < (int)test->ncols ; j++){

            if(sp_matrix_get(test,i,j)  !=  sp_matrix_get(loaded,i,j)){
                CU_FAIL("Loaded sparse matrix  from file did not match given values");
            }
        }
    }

    CU_ASSERT(1);
}


int main(){
    CU_pSuite pSuite1 = NULL;

    if(CUE_SUCCESS != CU_initialize_registry()){
        return CU_get_error();
    }

    pSuite1 = CU_add_suite("matrix_testing", init_suite, clean_suite);
    if (NULL == pSuite1) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if ((NULL == CU_add_test(pSuite1, "naive initialisation", naive_matrix_creation)) ||
        (NULL == CU_add_test(pSuite1, "naive sequential assignment", naive_matrix_sequential_assignment)) ||
        (NULL == CU_add_test(pSuite1, "naive random assignment", naive_matrix_random_backwards_assignment)) ||
        (NULL == CU_add_test(pSuite1, "naive to sparse transformation", naive_to_sparse)) ||
        (NULL == CU_add_test(pSuite1, "naive transpose", naive_sequential_transpose )) ||
        (NULL == CU_add_test(pSuite1, "naive addition", naive_addition )) ||
        (NULL == CU_add_test(pSuite1, "naive multiplication", naive_multiplication )) ||
        (NULL == CU_add_test(pSuite1, "naive file save", naive_save_load )) ||
        (NULL == CU_add_test(pSuite1, "sparse initialisation", sparse_matrix_creation )) ||
        (NULL == CU_add_test(pSuite1, "sparse sequential assignment", sparse_matrix_sequential_assignment )) ||
        (NULL == CU_add_test(pSuite1, "sparse random assignment", sparse_matrix_random_backwards_assignment )) ||
        (NULL == CU_add_test(pSuite1, "sparse diagonal assignment", sparse_assignment_diagonal )) ||
        (NULL == CU_add_test(pSuite1, "sparse to naive transformation", sparse_to_naive )) ||
        (NULL == CU_add_test(pSuite1, "sparse transpose", sparse_sequential_transpose )) ||
        (NULL == CU_add_test(pSuite1, "sparse addition", sparse_addition )) ||
        (NULL == CU_add_test(pSuite1, "sparse multiplication", sparse_multiplication )) ||
        (NULL == CU_add_test(pSuite1, "sparse file save", sparse_save_load )))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }


    CU_basic_run_tests();
  //  CU_basic_show_failures(CU_get_failure_list());
  return CU_get_error();

}



