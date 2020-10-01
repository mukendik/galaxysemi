#include <stdio.h>

// to compile me: g++ -o gtl_get_version gtl_get_version.c -lpthread -ldl -L. -lGTLenVision
// to execute me: LD_LIBRARY_PATH=. ./gtl_get_version

extern "C" { 
	int gtl_get(char* key, char* v);
}

int main()
{
	char v[256]="?";
	int r=gtl_get((char*)"lib_version", v); // r should be 0
	printf("GTL version: %s (%d)\n", v, r);
	return 0;
}
