#ifndef CONVERTER_TASKDATA_H
#define CONVERTER_TASKDATA_H

#include <time.h>
#include <QString>
#include <QDir>
#include <QTime>
#include <QMap>
#include <QVariant>
#include "task_properties.h"

// Forward declaration (in scheduler_engine.h)
class CGexMoTaskItem;

class GexMoFileConverterTaskData : public TaskProperties
{
public:
    GexMoFileConverterTaskData(QObject* parent);	// Constructor

    GexMoFileConverterTaskData& operator= (const GexMoFileConverterTaskData &copy);

    QString strTitle;           // Task title.
    QString strInputFolder;     // Folder to find files to convert
    QString strFileExtensions;  // list of files extensions to convert (eg: *.pcm;*.wat)
    QString strOutputFolder;    // Folder where converted files are created
    int     iFormat;            // STDF or CSV
    bool    bTimeStampName;     // 'true' if must include timestamp info in file name created.
    int     iFrequency;         // Task frequency. (use only for local task)
    int     iDayOfWeek;         // Day of Week to execute task (0= Monday, ...6=Sunday)
    int     iOnSuccess;         // What to do with files if successfuly converted
    QString strOutputSuccess;   // Where to store source files if successfuly converted
    int     iOnError;           // What to do with files if failed conversion
    QString strOutputError;     // Where to store source files if failed conversion

    int     mPriority;

    /*! \brief Update Private attributes :
           - update Private attributes from members
           - never in public slot
      */
    void UpdatePrivateAttributes();
};

#endif // CONVERTER_TASKDATA_H
