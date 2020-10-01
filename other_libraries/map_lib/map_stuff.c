#include <stdio.h>
#include "map_lib.h"

void display_name(struct map_t *m,char *first);

int main(int argc,char **argv) {
   struct map_t *test;
   test=map_create();
   map_set(test,"Clark","Kent");
   map_set(test,"Bruce","Wayne");
   map_set(test,"Hal","Jordan");

   display_name(test,"Bruce");
   display_name(test,"Hal");
   display_name(test,"Clark");

   printf("\n");

   map_set(test,"Clark","Savage");

   display_name(test,"Bruce");
   display_name(test,"Hal");
   display_name(test,"Clark");

   printf("\n");

   display_name(test,"Barry");

   map_set(test,"Barry","Allen");

   printf("\n");

   display_name(test,"Barry");
}

void display_name(struct map_t *m,char *first) {
   printf("%s %s\n",first,map_get(m,first));
}
