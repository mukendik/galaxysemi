#include <stdlib.h>
#include <stdio.h>

extern unsigned test_hash_functions(char** list_of_keys, const unsigned number_of_keys);

int main(int argc, char *argv[])
{
    char* tests[]={
    "2001 PTR test1 -1",
    "2002 PTR test2 -1",
    "2003 PTR test3 -1",
    "2003 PTR test4 -1",
    "3001 MPR test1 0",
    "3001 MPR test1 1",
    "3001 MPR test1 2",
    "3001 MPR test1 3",
    "3001 MPR test1 4",
    "3002 MPR test2 0",
    "3002 MPR test2 1"
    };

    unsigned r=test_hash_functions(tests, 11);
    printf("r = %d\n", r);

    return EXIT_SUCCESS;
}
