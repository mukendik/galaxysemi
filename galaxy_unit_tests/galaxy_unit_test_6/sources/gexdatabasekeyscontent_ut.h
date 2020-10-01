#ifndef GEXDATABASEKEYSCONTENT_UT_H
#define GEXDATABASEKEYSCONTENT_UT_H

//#include "db_datakeys.h"
#include <QString>

class GexDatabaseKeysContent_UT
{
public:
    GexDatabaseKeysContent_UT();
    bool RunTests();

    bool ut_InvalidStaticKeyName();
    bool ut_InvalidStaticKeyValue();
    bool ut_SetReadOnlyStaticKey();
    bool ut_SetStaticKey();
    bool ut_SetGetCustomKey();
    bool ut_TestNumberKey();
    bool ut_TestBooleanKey();
    bool ut_TestCharKey();
    bool ut_TestDateTimeKey();
    bool ut_LoadInvalidDbKeysFile(const QString &configFileName);
    bool ut_LoadValidDbKeysFile(const QString &configFileName);

};

#endif // GEXDATABASEKEYSCONTENT_UT_H
