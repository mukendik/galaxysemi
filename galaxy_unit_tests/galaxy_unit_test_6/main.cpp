#include <iostream>
#include <stdlib.h>

#include "sources/gexdatabasekeyscontent_ut.h"

using namespace std;

QString lUnknownkeyFile = QString("./dbkeys/unknown_key.gexdbkeys");
QString lValidkeysFile = QString("./dbkeys/valid_keys.gexdbkeys");

int main()
{
    cout << endl << "GexDatabaseKeysContent Unit Test 6" << endl;

    GexDatabaseKeysContent_UT KeysTest;
    bool bStatus = KeysTest.ut_InvalidStaticKeyName();
    if(bStatus)
        bStatus = KeysTest.ut_InvalidStaticKeyValue();
    if(bStatus)
        bStatus = KeysTest.ut_SetStaticKey();
    if(bStatus)
        bStatus = KeysTest.ut_SetReadOnlyStaticKey();
    if(bStatus)
        bStatus = KeysTest.ut_SetGetCustomKey();
    if(bStatus)
        bStatus = KeysTest.ut_TestNumberKey();
    if(bStatus)
        bStatus = KeysTest.ut_TestBooleanKey();
    if(bStatus)
        bStatus = KeysTest.ut_TestCharKey();
    if(bStatus)
        bStatus = KeysTest.ut_TestDateTimeKey();
    if(bStatus)
        bStatus = KeysTest.ut_LoadInvalidDbKeysFile(lUnknownkeyFile);
    if(bStatus)
        bStatus = KeysTest.ut_LoadValidDbKeysFile(lValidkeysFile);

    if(bStatus)
        return EXIT_SUCCESS;

    return EXIT_FAILURE;
}

