#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"

/*
static int callback(void* NotUsed, int argc, char **argv, char **azColName)
{
    int i=0;
    for(i=0; i<argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}
*/

class GSElapsedTimer
{
public:

    GSElapsedTimer()
    {
        LARGE_INTEGER frequency;
        if (!QueryPerformanceFrequency(&frequency)) {
            mCounterFrequency = 0;
        } else {
            mCounterFrequency = frequency.QuadPart;
        }

        mStart = 0;
    }

    void    start()
    {
        LARGE_INTEGER counter;

        if (QueryPerformanceCounter(&counter))
            mStart = counter.QuadPart;
    }

    long    elapsed()
    {
        long lElapsed = -1;

        if (mStart == 0)
            return lElapsed;

        LARGE_INTEGER counter;

        QueryPerformanceCounter(&counter);

        long lTicks = counter.QuadPart - mStart;

        if (mCounterFrequency > 0) {
            // QueryPerformanceCounter uses an arbitrary frequency
            LONGLONG seconds = lTicks / mCounterFrequency;
            LONGLONG nanoSeconds = (lTicks - seconds * mCounterFrequency) * 1000000000 / mCounterFrequency;
            return (long) ((seconds * 1000000000 + nanoSeconds) / 1000000) ;
        } else {
            // GetTickCount(64) return milliseconds
            return lTicks;
        }

    }

    long    restart()
    {
        long lElapsed = elapsed();

        start();

        return lElapsed;
    }

private:

    LONGLONG    mStart;
    LONGLONG    mCounterFrequency;

};

char *zErrMsg = 0;
sqlite3 *db=0;

int check_rc(int rc)
{
    if (rc==SQLITE_OK)
        return 0;
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    sqlite3_close(db);
    exit(rc);
    //return(rc);
}

#define DRAND(x,y) x+((double)rand()/(double)RAND_MAX)*(y-x)

int test_count      = 100;
int tuning_count    = 100;
int bench_count     = 1;

char buf[2048]="";
int rc=0;
int flushTime=0;

int write_xml(const char * filename, bool close_at_tuning)
{
    FILE* xml = NULL;
    for (int i = 0; i < tuning_count; ++i)
    {
        if (i == 0)
            xml=fopen(filename, "w");
        else if (close_at_tuning)
            xml=fopen(filename, "a");

        if (xml)
        {
            for (int j=0; j < test_count; ++j)
            {
                sprintf(buf, "<dpat><site>%d</site> <part_id>A%d<part_id>" \
                        "<test id=\"%d\">" \
                            "<limit units=\"V\" count=\"2\">" \
                                "<low idx=\"0\">%f</low>" \
                                "<high idx=\"0\">%f</high>" \
                            "</limit>" \
                        "</test>" \
                        "</dpat>\n", i*j, rand(), rand(), DRAND(0,1), DRAND(0,1) );

                fprintf(xml, buf);
            }

            if (close_at_tuning)
                fclose(xml);
        }
        else
        {
            printf("Failed to open xml file");
            return EXIT_FAILURE;
        }
    }
    return 0;
}

int insert_sqlite(bool backup_on_tuning, bool backup_at_end)
{
    for (int i=0; i < tuning_count; ++i)
    {
        for (int j=0; j < test_count; ++j)
        {
            sprintf(buf, "insert into limits VALUES(%d, 'PART%d', %d, %f, %f)", i*j, rand(), rand(), DRAND(0,1), DRAND(0,1) );
            rc = sqlite3_exec(db, buf, 0, 0, &zErrMsg);
            check_rc(rc);
        }

        if (backup_on_tuning)
        {
            GSElapsedTimer lET;
            lET.start();
            sqlite3 *dest = 0;
            rc = sqlite3_open("backup.sqlite", &dest);
            check_rc(rc);
            sqlite3_backup *dbbackup = sqlite3_backup_init(dest, "main", db, "main");
            if (!dbbackup)
            {
                rc = sqlite3_errcode(dest);
                check_rc(rc);
            }
            rc=sqlite3_backup_step(dbbackup, -1); // All pages ?
            if (rc!=SQLITE_DONE)
            {
                printf("sqlite3_backup_step failed\n");
                exit(-1);
            }
            sqlite3_backup_finish(dbbackup);
            flushTime += lET.elapsed();
        }
    }

    if (backup_at_end)
    {
        GSElapsedTimer lET;
        lET.start();
        sqlite3 *dest = 0;
        rc = sqlite3_open("backup.sqlite", &dest);
        check_rc(rc);
        sqlite3_backup *dbbackup = sqlite3_backup_init(dest, "main", db, "main");
        if (!dbbackup)
        {
            rc = sqlite3_errcode(dest);
            check_rc(rc);
        }
        rc=sqlite3_backup_step(dbbackup, -1); // All pages ?
        if (rc!=SQLITE_DONE)
        {
            printf("sqlite3_backup_step failed\n");
            exit(-1);
        }
        sqlite3_backup_finish(dbbackup);
        flushTime += lET.elapsed();
    }

    return 0;
}

int create_sqlite_db(char* dbpath)
{
    rc = sqlite3_open(dbpath, &db);
    if( rc )
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return(1);
    }

    rc = sqlite3_exec(db, "drop table if exists limits", 0, 0, &zErrMsg);
    if( rc!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return(1);
    }

    rc = sqlite3_exec(db, // primary key ?????
      "create table limits (site integer, part_id varchar, test_id integer, LL number, HL number)", 0, 0, &zErrMsg);
    check_rc(rc);
    return rc;
}

int main(int argc, char** argv)
{
    for (int iArg = 0; iArg < argc; ++iArg)
    {
        if (strnicmp(argv[iArg], "--benchcount=", 13) == 0)
            bench_count = atoi(argv[iArg]+13);
        else if (strnicmp(argv[iArg], "--tuningcount=", 14) == 0)
            tuning_count = atoi(argv[iArg]+14);
        else if (strnicmp(argv[iArg], "--testcount=", 12) == 0)
            test_count = atoi(argv[iArg]+12);
    }

    if (bench_count == 0)
    {
        printf("bench count equal to 0, the program will now exit.");
        return EXIT_FAILURE;
    }
    if (test_count == 0)
    {
        printf("test count equal to 0, the program will now exit.");
        return EXIT_FAILURE;
    }
    if (tuning_count == 0)
    {
        printf("tuning count equal to 0, the program will now exit.");
        return EXIT_FAILURE;
    }

    GSElapsedTimer elapsedTimer;

    FILE * bench_file = fopen("traceability_bench.txt", "w");

    if (bench_file == NULL)
        printf("failed to write traceability bench file\n");

    printf("Tuning count: %d\n", tuning_count);
    printf("Test count: %d\n", test_count);
    printf("Bench count: %d\n\n", bench_count);

    if (bench_file)
    {
        fprintf(bench_file, "Tuning count: %d\n", tuning_count);
        fprintf(bench_file, "Test count: %d\n", test_count);
        fprintf(bench_file, "Bench count: %d\n\n", bench_count);
    }

    printf("Starting Sqlite insertion (synchronous=FULL)\n");

    if (bench_file)
        fprintf(bench_file, "Starting Sqlite insertion (synchronous=FULL)\n");

    elapsedTimer.start();
    for (int i= 1; i <= bench_count; ++i)
    {
        create_sqlite_db((char*)"data-full.sqlite");
        rc = sqlite3_exec(db, "PRAGMA synchronous=FULL", 0, 0, &zErrMsg);
        check_rc(rc);
        insert_sqlite(false, false);
        sqlite3_close(db);

        printf("Benchmark executed %d/%d\n", i, bench_count);
    }
    printf("Sqlite insertion (Syncrhonous=FULL) done: %ld ms\n\n", (long)elapsedTimer.elapsed()/bench_count);
    if (bench_file)
        fprintf(bench_file, "Sqlite insertion (Syncrhonous=FULL) done: %ld ms\n\n", (long)elapsedTimer.elapsed()/bench_count);

    printf("Starting Sqlite insertion (synchronous=NORMAL)\n");
    if (bench_file)
        fprintf(bench_file, "Starting Sqlite insertion (synchronous=NORMAL)\n");
    elapsedTimer.restart();
    for (int i= 1; i <= bench_count; ++i)
    {
        create_sqlite_db((char*)"data-normal.sqlite");
        rc = sqlite3_exec(db, "PRAGMA synchronous=NORMAL", 0, 0, &zErrMsg);
        check_rc(rc);
        insert_sqlite(false, false);
        sqlite3_close(db);

        printf("Benchmark executed %d/%d\n", i, bench_count);
    }
    printf("Sqlite insertion (synchronous=NORMAL) done: %ld ms\n\n", (long)elapsedTimer.elapsed()/bench_count);
    if (bench_file)
        fprintf(bench_file, "Sqlite insertion (synchronous=NORMAL) done: %ld ms\n\n", (long)elapsedTimer.elapsed()/bench_count);

    printf("Starting Sqlite insertion (synchronous=OFF)\n");
    if (bench_file)
        fprintf(bench_file, "Starting Sqlite insertion (synchronous=OFF)\n");
    elapsedTimer.restart();
    for (int i= 1; i <= bench_count; ++i)
    {
        create_sqlite_db((char*)"data-off.sqlite");
        rc = sqlite3_exec(db, "PRAGMA synchronous=OFF", 0, 0, &zErrMsg);
        check_rc(rc);
        insert_sqlite(false, false);
        sqlite3_close(db);

        printf("Benchmark executed %d/%d\n", i, bench_count);
    }
    printf("Sqlite insertion (synchronous=OFF) done: %ld ms\n\n", (long)elapsedTimer.elapsed()/bench_count);
    if (bench_file)
        fprintf(bench_file, "Sqlite insertion (synchronous=OFF) done: %ld ms\n\n", (long)elapsedTimer.elapsed()/bench_count);

    printf("Starting Sqlite insertion (In-memory flushing db at each tuning)\n");
    if (bench_file)
        fprintf(bench_file, "Starting Sqlite insertion (In-memory flushing db at each tuning)\n");
    flushTime = 0;
    elapsedTimer.restart();
    for (int i= 1; i <= bench_count; ++i)
    {
        create_sqlite_db((char*)":memory:");
        insert_sqlite(true, true);
        sqlite3_close(db);

        printf("Benchmark executed %d/%d\n", i, bench_count);
    }
    printf("Sqlite insertion (In-memory flushing db at each tuning) done: %ld ms including %ld ms for db flushing\n\n",
           (long)elapsedTimer.elapsed()/bench_count, (long) flushTime/bench_count);
    if (bench_file)
        fprintf(bench_file,
                "Sqlite insertion (In-memory flushing db at each tuning) done: %ld ms including %ld ms for db flushing\n\n",
                (long)elapsedTimer.elapsed()/bench_count, (long) flushTime/bench_count);

    printf("Starting Sqlite insertion (In-memory flushing db at the end)\n");
    if (bench_file)
        fprintf(bench_file, "Starting Sqlite insertion (In-memory flushing db at the end)\n");
    flushTime = 0;
    elapsedTimer.restart();
    for (int i= 1; i <= bench_count; ++i)
    {
        create_sqlite_db((char*)":memory:");
        insert_sqlite(false, true);
        sqlite3_close(db);

        printf("Benchmark executed %d/%d\n", i, bench_count);
    }
    printf("Sqlite insertion (In-memory flushing db at the end) done: %ld ms including %ld ms for db flushing\n\n",
           (long)elapsedTimer.elapsed()/bench_count, (long) flushTime/bench_count);
    if (bench_file)
        fprintf(bench_file,
                "Sqlite insertion (In-memory flushing db at the end) done: %ld ms including %ld ms for db flushing\n\n",
                (long)elapsedTimer.elapsed()/bench_count, (long) flushTime/bench_count);

    printf("Starting XML generation (open and close file at each tuning)\n");
    if (bench_file)
        fprintf(bench_file, "Starting XML generation (open and close file at each tuning)\n");
    elapsedTimer.restart();
    for (int i= 1; i <= bench_count; ++i)
    {
        write_xml("data.xml", true);

        printf("Benchmark executed %d/%d\n", i, bench_count);
    }
    printf("XML generation (open and close file at each tuning) done: %ld ms\n\n", (long)elapsedTimer.elapsed()/bench_count);
    if (bench_file)
        fprintf(bench_file,
                "XML generation (open and close file at each tuning) done: %ld ms\n\n", (long)elapsedTimer.elapsed()/bench_count);

    printf("Starting XML generation (close file at the end)\n");
    if (bench_file)
        fprintf(bench_file, "Starting XML generation (close file at the end)\n");
    elapsedTimer.restart();
    for (int i= 1; i <= bench_count; ++i)
    {
        write_xml("data.xml", true);

        printf("Benchmark executed %d/%d\n", i, bench_count);
    }
    printf("XML generation (close file at the end) done: %ld ms\n\n", (long)elapsedTimer.elapsed()/bench_count);
    if (bench_file)
        fprintf(bench_file, "XML generation (close file at the end) done: %ld ms\n\n", (long)elapsedTimer.elapsed()/bench_count);

    if (bench_file)
        fclose(bench_file);

    printf("Quitting...\n");

    return EXIT_SUCCESS; //a.exec();
}
