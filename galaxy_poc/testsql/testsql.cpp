// -------------------------------------------------------------------------- //
// testsql.cpp
// -------------------------------------------------------------------------- //
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <iostream>

// -------------------------------------------------------------------------- //
// main
// -------------------------------------------------------------------------- //
int main(int argc, char* argv[])
{
    const unsigned int QMYSQL = 0;
    const unsigned int QOCI = 1;
    unsigned int driver;

    if (argc == 4)
    {
        driver = QMYSQL;
    }
    else if (argc == 5)
    {
        driver = QOCI;
    }
    else
    {
        std::cout << "Usage: " << argv[0] << " <host> <user> <pass> [sid]"
                  << std::endl;
        return EXIT_SUCCESS;
    }

    QSqlDatabase db =
        QSqlDatabase::addDatabase((driver == QMYSQL) ? "QMYSQL" : "QOCI");
    db.setHostName(argv[1]);
    db.setUserName(argv[2]);
    db.setPassword(argv[3]);
    if (argc == 5)
    {
        db.setDatabaseName(argv[4]);
    }
    if (! db.open())
    {
        QSqlError error = db.lastError();
        std::cout << error.text().toUtf8().data() << std::endl;
        return EXIT_FAILURE;
    }

    QSqlQuery query =
        db.exec((driver == QMYSQL) ?
                "show databases" :
                "select * from all_users");
    QSqlError error = query.lastError();
    if (error.isValid())
    {
        std::cout << error.text().toUtf8().data() << std::endl;
        return EXIT_FAILURE;
    }
    while (query.next())
    {
        std::cout << query.value(0).toString().toUtf8().data() << std::endl;
    }

    return EXIT_SUCCESS;
}
