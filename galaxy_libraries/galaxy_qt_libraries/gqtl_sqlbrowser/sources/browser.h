
#ifndef BROWSER_H
#define BROWSER_H

#include <QWidget>
#include <QCompleter>
#include <QThread>
#include <QTextEdit>
#include <QClipboard>
#include "ui_browserwidget.h"

class ConnectionWidget;
QT_FORWARD_DECLARE_CLASS(QTableView)
QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QTextEdit)
QT_FORWARD_DECLARE_CLASS(QSqlError)

namespace GS
{
    namespace QtLib
    {
        class SqlBrowserPrivate: public QWidget, private Ui::SqlBrowser
        {
            Q_OBJECT

            QCompleter *mCompleter;
            QThread mThread;
            QTextEdit* sqlEdit;
        public:
            SqlBrowserPrivate(QWidget *parent = 0);
            virtual ~SqlBrowserPrivate();

            QSqlError addConnection(
                    const QString &driver, const QString &dbName, const QString &host,
                    const QString &user, const QString &passwd, int port = -1
                    );

            void insertRow();
            void deleteRow();
            void updateActions();

        public slots:
            void refreshDBlist();
            void exec();
            void showTable(const QString &table);
            void showMetaData(const QString &table);
            // just show the GUI dialog
            void addConnection();
            void currentChanged();
            void about();
            void on_insertRowAction_triggered() { insertRow(); }
            void on_deleteRowAction_triggered() { deleteRow(); }
            void on_copyAction_triggered();
            void on_connectionWidget_tableActivated(const QString &table);
            void on_connectionWidget_metaDataRequested(const QString &table) { showMetaData(table); }
            void on_submitButton_clicked();
            void on_clearButton_clicked();
            void on_save_sqltextedit();

        signals:
            void statusMessage(const QString &message);
        };
    }
}
#endif
