#ifndef MY_SERVICE_H
#define MY_SERVICE_H

#include "my_core_task.h"
#include "gqtl_service.h"

class MyService : public QtService<QCoreApplication>
{
public:

    MyService(int argc, char **argv, const QString &name);
    ~MyService();

protected:

    void			start();			// Function which is called when the service starts
    void			stop();				// Function which is called when the service stops
    void			pause();			// Function which is called when the service goes to pause
    void			resume();			// Function which is called when the service resumes
    void            processCommand(int code);

    bool            GetTaskInfo(int lTaskID, int &lTaskType, int &lDuration) const;

private:

    MyCoreTask  mCoreTask;
};

#endif // MY_SERVICE_H
