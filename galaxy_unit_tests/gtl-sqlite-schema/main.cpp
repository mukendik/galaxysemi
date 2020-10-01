/******************************************************************************!
 * \file main.cpp
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

#ifdef __cplusplus
extern "C" {
#endif

int gtl_CreateSchema(sqlite3* lDB);
extern char* zErrMsg;

#ifdef __cplusplus
}
#endif

/******************************************************************************!
 * \fn callback
 ******************************************************************************/
static unsigned int gCallbackCount = 0;
static int callback(void* /*data*/, int argc, char** argv, char** azColName)
{
    int i;

    // fprintf(stderr, "%s: ", (const char*) data);
    for (i = 0; i < argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    ++gCallbackCount;
    return 0;
}

/******************************************************************************!
 * \fn main
 ******************************************************************************/
int main()
{
    sqlite3* lDB = NULL;
    char* lErr = NULL;
    int rc;

    rc = sqlite3_open(":memory:", &lDB);
    if (rc != SQLITE_OK)
    {
        printf("sqlite3_open: %d\n", rc);
        exit(EXIT_FAILURE);
    }
    rc = gtl_CreateSchema(lDB);
    if (rc != 0)
    {
        printf("create schema: %s\n", zErrMsg);
        exit(EXIT_FAILURE);
    }
    rc = sqlite3_exec(lDB,
                      "SELECT name"
                      " FROM sqlite_master"
                      " WHERE type = 'table'"
                      " ORDER BY 1"
                      ";",
                      callback, 0, &lErr);
    if (rc != 0)
    {
        printf("query exec: %d\n", rc);
        exit(EXIT_FAILURE);
    }
    printf("Total = %u\n", gCallbackCount);

    sqlite3_free(lErr);
    sqlite3_free(zErrMsg);
    sqlite3_close(lDB);

    return EXIT_SUCCESS;
}
