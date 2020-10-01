#include "my_service.h"

int main(int argc, char *argv[])
{
    MyService lService(argc, argv, "Galaxy POC service");

    return lService.exec();
}
