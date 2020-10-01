// #ifdef GTLDEBUG
#ifdef CPPSTL

#include <iomanip> // for setprecision
#include <iostream>
#include <fstream> // for ofstream
#include <ios>

extern "C"
{

static std::ofstream sDump;

char* OpenDump(char* lFileName)
{
    sDump.open(lFileName, std::ofstream::out | std::ofstream::app);
    if (sDump.is_open())
        return (char*)"error: cannot open dump";
    return (char*)"ok";
}

void OutputStringToDump(char* lString)
{
    if (lString)
        sDump<<lString;
}

void OutputFloatToDump(float lFloat, int lPrecision)
{
    sDump<<std::setprecision(lPrecision)<<lFloat;
}

void OutputIntToDump(int lInt)
{
    sDump<<lInt;
}

char DumpOpened()
{
    if (sDump.is_open())
        return 'Y';
    return 'N';
}

void OutputLongToDump(long lVar)
{
    sDump<<lVar;
}

void CloseDump()
{
    sDump.close();
}

} // extern C
#else

    extern "C"
    {

    #include <stdlib.h>
    #include <stdio.h>

    static FILE* sDump=0;
    char* OpenDump(char* lFileName)
    {
        sDump=fopen(lFileName, "a+");
        if (sDump)
            return (char*)"error: cannot open dump";
        return (char*)"ok";
    }

    void OutputStringToDump(char* lString)
    {
        if (lString && sDump)
            fprintf(sDump, "%s", lString);
    }

    void OutputFloatToDump(float lFloat, int lPrecision)
    {
        //sDump<<std::setprecision(lPrecision)<<lFloat;
        if (sDump)
            fprintf(sDump, "%.*e", lPrecision, lFloat); // todo : find a way to set precision from a variable
    }

    void OutputIntToDump(int lInt)
    {
        if (sDump)
            fprintf(sDump, "%d", lInt);
    }

    char DumpOpened()
    {
        if (sDump)
            return 'Y';
        return 'N';
    }

    void OutputLongToDump(long lVar)
    {
        if (sDump)
            fprintf(sDump, "%ld", lVar);
    }

    void CloseDump()
    {
        fclose(sDump);
    }
    } // extern C
#endif
