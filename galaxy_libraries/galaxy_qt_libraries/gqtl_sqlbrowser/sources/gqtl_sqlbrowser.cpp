#include <QWidget>
#include <QSqlDatabase>
#include "gqtl_sqlbrowser.h"
#include "browser.h"

static GS::QtLib::SqlBrowser* sInstance=0;

namespace GS
{
    namespace QtLib
    {
        SqlBrowser::SqlBrowser(QWidget *p): QWidget(p)
        {
            mPrivate = new SqlBrowserPrivate(p);
            setLayout(new QVBoxLayout(this));
            layout()->addWidget(mPrivate);
        }

        SqlBrowser::~SqlBrowser()
        {
            if (mPrivate)
                delete mPrivate;
        }

        void SqlBrowser::Refresh()
        {
            mPrivate->refreshDBlist();
        }
        /*
        void SqlBrowser::NewDBConnection(QMap< QString, QVariant> params)
        {
            QStringList cs=QSqlDatabase::connectionNames();
            foreach(QString c, cs)
            {
                //QSqlDatabase
                        //mPrivate->
            }
        }
        */
    }
}


extern "C" LIBSHARED_EXPORT
GS::QtLib::SqlBrowser*
GetInstance(QWidget* parent, QString &o)
{
    o="GetInstance with parent "+(parent?parent->objectName():QString("null") );

    if (!sInstance)
    {
        sInstance=new GS::QtLib::SqlBrowser(parent);
        sInstance->setObjectName("SqlBrowser");
    }

    return sInstance;
}
