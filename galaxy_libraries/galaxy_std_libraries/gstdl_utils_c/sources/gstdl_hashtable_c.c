// minimalist hashtable

#include <stdlib.h>
#include <stdio.h>
#include <string.h> // for strlen
#include <math.h> // for pow
#include <limits.h>

#if defined __unix__ || __APPLE__&__MACH__
#include <limits.h>
#endif

// some hash functions: ALL must have 2 params: 1 the string to hash, 2 the size of the hashtable

#define A 54059 /* a prime */
#define B 76963 /* another prime */
#define C 86969 /* yet another prime */
unsigned hash_function_1(const char* s, const unsigned size_of_table)
{
   unsigned h = 31 /* also prime */;
   while (*s)
   {
     h = (h * A) ^ (s[0] * B);
     s++;
   }
   //return h; // or return h % C;
   return size_of_table>0?h%size_of_table:h;
}

//#define SIZE 10
unsigned hash_function_2(const char* word, const unsigned size_of_table)
{
   int seed = 131;
   unsigned long hash = 0;
   unsigned length=strlen(word);
   unsigned i=0;
   for(i = 0; i < length; i++)
   {
      hash = (hash * seed) + word[i];
   }

   //return hash % SIZE;
   //return hash % size_of_table;
   return size_of_table>0?hash%size_of_table:hash;
}

// returns the index of entry
unsigned hash_function_3(const char* word, const unsigned size_of_table)
{
    int sum = 0;
    unsigned length=strlen(word);
    unsigned k=0;
    for (k = 0; k < length; k++)
            sum = sum + (int)word[k];
    //return  sum % size_of_table;
    if (size_of_table>1)
        return sum%size_of_table;

    return sum;
}

unsigned int hash_function_4(const char* str, const unsigned size_of_table)
{
    unsigned int b    = 378551;
    unsigned int a    = 63689;
    unsigned int hash = 0;
    unsigned i=0;
    unsigned length=strlen(str);
    for(i = 0; i < length; i++)
    {
        hash = hash * a + str[i];
        a    = a * b;
    }
    //return (hash & 0x7FFFFFFF);
    //return (hash & 0x7FFFFFFF) % size_of_table;
    if (size_of_table>0)
        return ((hash<<0x7FFFFFFF)%size_of_table);
    return (hash<<0x7FFFFFFF);
}

unsigned int hash_function_5(const char* str, const unsigned size_of_table)
{
    unsigned int hash = 1315423911;
    unsigned i=0;
    unsigned length=strlen(str);
    for(i = 0; i < length; i++)
      {
          hash ^= ((hash << 5) + str[i] + (hash >> 2));
      }
    //return (hash & 0x7FFFFFFF) % size_of_table;
    return size_of_table>0?((hash<<0x7FFFFFFF)%size_of_table):(hash<<0x7FFFFFFF);
}

unsigned hash_function_6(const char* word, const unsigned size_of_table)
{
    int result = 0;
    unsigned i=0;
    unsigned length=strlen(word);
    for(i = 0; i < length; ++i)
    {
        result += word[i] * pow(31, i);
    }
    //return result;
    if (size_of_table>0)
        return result%size_of_table;
    return result;
}

unsigned hash_function_7(const char* x, const unsigned size_of_table)
{
    // http://en.wikipedia.org/wiki/Universal_hashing
    int a=1;
    unsigned i=0;
    //int p=(pow(2,61)-1);
    unsigned p=((unsigned)pow(2,128)-1);
    unsigned length=strlen(x);
    int h=x[0];
    for ( i=1 ; i<length; i++)
        h = ((h*a) + x[i]) % p;
    if (size_of_table>1)
        return h%size_of_table;
    return h;
}

unsigned hash_function_8(const char* word, const unsigned size_of_table)
{
    int sum = 0;
    unsigned length=strlen(word);
    unsigned k=0;
    for (k=0; k<length; k++)
        sum = sum + (int)word[k];
    //return  sum % size_of_table;
    if (size_of_table>1)
        return sum%size_of_table;

    return sum;
}

/* Perl's hash function from libcfu */
unsigned
hash_function_9(const char* key, const unsigned size_of_table)  //size_t length)
{
    register size_t i = strlen(key);    //length;
    //register u_int hv = 0; /* could put a seed here instead of zero */
    register unsigned hv = 0; /* could put a seed here instead of zero */
    register const unsigned char* s = (const unsigned char*) key;
    while (i--)
    {
        hv += *s++;
        hv += (hv << 10);
        hv ^= (hv >> 6);
    }
    hv += (hv << 3);
    hv ^= (hv >> 11);
    hv += (hv << 15);

    if (size_of_table>1)
        return hv%size_of_table;
    return hv;
}

unsigned hash_function_10(const char* key, const unsigned size_of_table)  //size_t length)
{
    register size_t i = strlen(key);    //length;
    //register u_int hv = 0; /* could put a seed here instead of zero */
    register unsigned hv = 0; /* could put a seed here instead of zero */
    register const unsigned char* s = (const unsigned char*) key;
    while (i--)
    {
        hv += *s++;
        hv += (hv << 10);
        hv ^= (hv >> 6);
    }
    hv += (hv << 3);
    hv ^= (hv >> 11);
    hv += (hv << 15);

    if (size_of_table>0)
        return hv%size_of_table;
    return hv/10000000;
}

unsigned hash_function_11(const char *key, const unsigned size_of_table) //size_t length)
{
    size_t i = strlen(key); //length;
    //u_int hash = 0;
    unsigned hash=0;
    char *s = (char *)key;
    while (i--)
        hash = hash * 33 + *s++;
    if (size_of_table>0)
        return hash%size_of_table;
    return hash;
}

typedef unsigned (*hash_func_t)(const char*, const unsigned);

// iterate in order to optimize a perfect hash function
// returns the best ratio to use in order to minimize index range
/*
unsigned optimize_hash_function(hash_func_t *f)
{
    unsigned ratio=10;
    int r=0;
    for (r=0; r<10; r++)
    {

    }
}
*/

// return 'y' or 'n'
char is_hash_function_perfect(unsigned *list_of_indexes, const unsigned number_of_keys)
{
    unsigned k, l;
    for (k=0; k<number_of_keys; k++)
    {
        l=0;
        for (l=k+1; l<number_of_keys; l++)
        {
            if (list_of_indexes[k]==list_of_indexes[l])
            {
                printf("Hash function not perfect: %u vs %u\n", k, l);
                return 'n';
                //goto next_function;
                //break;
                //return -1;
            }
        }
    }
    return 'y';
}

// test all hash functions using given dictionary
unsigned test_hash_functions(char** list_of_keys, const unsigned number_of_keys)
{
    hash_func_t functions[]={
        hash_function_1, hash_function_2,
        hash_function_3, hash_function_4, hash_function_5, hash_function_6, hash_function_7,
        hash_function_8, hash_function_9,
        hash_function_10, hash_function_11
    };

    if (number_of_keys<1)
        return -1;
    unsigned i=0;
    unsigned j=0;
    unsigned* list_of_indexes=malloc(sizeof(unsigned)*number_of_keys);
    for (j=0; j<10; j++) // 5 hash functions
    {
        printf("Hash function %d:", j);
        for (i=0; i<number_of_keys; i++)
        {
            //list_of_indexes[i]=hash_function_1(list_of_keys[i], number_of_keys);
            list_of_indexes[i]=functions[j](list_of_keys[i], 0); // number_of_keys);
            printf("%u ", (unsigned)list_of_indexes[i]);
        }
        printf("\n");

        unsigned lMin=UINT_MAX, lMax=0;
        unsigned k=0;
        for (k=0; k<number_of_keys; k++)
        {
            if (lMin>list_of_indexes[k])
                lMin=list_of_indexes[k];
            if (lMax<list_of_indexes[k])
                lMax=list_of_indexes[k];
            unsigned l=0;
            for (l=k+1; l<number_of_keys; l++)
            {
                if (list_of_indexes[k]==list_of_indexes[l])
                {
                    printf("Hash function %u not perfect: %u vs %u\n", j, k, l);
                    //goto next_function;
                    break;
                    //return -1;
                }
            }
        }
        //if (k==(number_of_keys-1))
            printf("Min=%u Max=%u delta=%u\n", lMin, lMax, lMax-lMin);

    }

    free(list_of_indexes);

    return 0;
}
