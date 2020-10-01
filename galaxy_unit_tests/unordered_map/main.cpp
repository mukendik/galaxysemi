#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <tr1/unordered_map>

int main()
{
    std::cout<<"STL TR1 hash table unit test"<<std::endl;
    std::cout<<"sizeof unsigned="<<sizeof(unsigned)<<" sizeof void*="<<sizeof(void*)<<  std::endl;

    //std::tr1::unordered_map<const char*, void*> lHashmap; // char* key does not work
    std::tr1::unordered_map<std::string, void*> lHashmap;
    std::stringstream lSS;

    const unsigned n=1000000;
    std::cout<<"Inserting..."<<std::endl;
    for (unsigned i=0; i<n; i++)
    {
        lSS.str(""); //lSS.str(); lSS.flush();
        lSS<<"test"<<i;
        //lKey=lSS.str();
        lHashmap[lSS.str()]=(void*)i;
    }

    std::cout<<"Hashmap size after insertion:"<<lHashmap.size() <<" maxsize="<<lHashmap.max_size() <<std::endl;
    if (lHashmap.size()!=n)
    {
        std::cout<<"Unexpected size:"<<lHashmap.size()<<std::endl;
        if (lHashmap.size()>0)
            std::cout<<"element 0="<<(unsigned)lHashmap[lSS.str()]<<std::endl;
        return EXIT_FAILURE;
    }

    std::cout<<"checking hash content..."<<std::endl;
    for (unsigned i=0; i<n; i++)
    {
        lSS.str(""); //lSS.flush();
        lSS<<"test"<<i;
        if (lHashmap.find(lSS.str()) == lHashmap.end())
        {
            std::cout<<"Unfindable key "<< lSS.str() <<std::endl;
            return EXIT_FAILURE;
        }
        void* p=lHashmap[lSS.str()];
        if(p!=(void*)i)
        {
            std::cout<<"unexpected value for key '"<<lSS.str()<<"':"<< i <<" pointer:"<< (void*)i <<std::endl;
            printf("pointer=%p NULL=%p\n", p, (void*)NULL);
            return EXIT_FAILURE;
        }
    }

    std::cout<<"Success"<<std::endl;
    return EXIT_SUCCESS;
}
