#include "browser.h"
#include "textedit.h"
#include "qsqlconnectiondialog.h"

#include <gqtl_log.h>
#include <QtGui>
#include <QtSql>
#include <QShortcut>
#include <QCompleter>
#include <QDir>

namespace GS
{
namespace QtLib
{

SqlBrowserPrivate::SqlBrowserPrivate(QWidget *parent)
    : QWidget(parent)
{
    GSLOG(5, "SqlBrowserPrivate::SqlBrowserPrivate");

    setupUi(this);

    sqlEdit=new TextEdit(mQueryGroupBox);
    mQueryGroupBox->layout()->addWidget(sqlEdit);
    //mQueryGroupBox->insertChild(new TextEdit(mQueryGroupBox));

    QStringList wordList;
    wordList << "select" << "insert" << "update" << "delete" << "distinct" << "where" << "from";
     mCompleter = new QCompleter(wordList, sqlEdit);
     mCompleter->setCaseSensitivity(Qt::CaseInsensitive);
     mCompleter->setCompletionMode(QCompleter::PopupCompletion);
     ((TextEdit*)sqlEdit)->setCompleter(mCompleter);
    //completer->setWrapAround(false);
     //QObject::connect(mCompleter, SIGNAL(activated(QString)),
       //               this, SLOT(insertCompletion(QString)));


    QShortcut* sc1=new QShortcut(Qt::CTRL+Qt::Key_S, sqlEdit);
            sc1->setEnabled(true);
            QObject::connect(sc1, SIGNAL(activated()), this, SLOT(on_save_sqltextedit()));

    QShortcut* sc2=new QShortcut(Qt::CTRL+Qt::Key_Return, sqlEdit);
        sc2->setEnabled(true);
        QObject::connect(sc2, SIGNAL(activated()), this, SLOT(on_submitButton_clicked()));

    table->addAction(insertRowAction);
    table->addAction(deleteRowAction);
    table->addAction(actionCopy);
    QObject::connect(actionCopy, SIGNAL(triggered()), this, SLOT(on_copyAction_triggered()) );

    if (QSqlDatabase::drivers().isEmpty())
        QMessageBox::information(this, tr("No database drivers found"),
                                 tr("This feature requires at least one Qt database driver. " ));


     QFile f(QDir::homePath()+"/GalaxySemi/temp/sqlbrowser.txt");
     if (f.open(QIODevice::ReadOnly))
     {
        QString c=QString(f.readAll());
        sqlEdit->setText(c);
        f.close();
     }

     QObject::connect(mRefreshPushButton, SIGNAL(released()), this, SLOT(refreshDBlist()));

    emit statusMessage(tr("Ready."));
}

SqlBrowserPrivate::~SqlBrowserPrivate()
{
    on_save_sqltextedit();
    if (mCompleter)
            delete mCompleter;
    mCompleter=0;
}

void SqlBrowserPrivate::exec()
{
    emit statusMessage("Executing query...");
    mQueryGroupBox->setTitle("Executing query...");
    QTime t;
    QSqlQueryModel *model = new QSqlQueryModel(table);
    t.start();

    QTextCursor c=sqlEdit->textCursor();
    QTextBlock tb=sqlEdit->textCursor().block();

    try
    {
        model->setQuery(
                    QSqlQuery(
                         tb.text()   //sqlEdit->toPlainText()
                        , connectionWidget->currentDatabase()));
        table->setModel(model);
    }
    catch(const std::bad_alloc &e)
    {
        emit statusMessage("errror : bad alloc exception");
        return;
    }
    float nMS=(float)t.elapsed();

    QString m;
    if (model->lastError().type() != QSqlError::NoError)
        m=model->lastError().text();
    else
    {
        if (model->query().isSelect())
            m="Query OK executed in "+QString::number(nMS/1000.f)+"s     "
                +QString::number(model->query().size())+" rows returned and "
                +QString::number(model->query().numRowsAffected())+" rows affected.";
        else
            m=QString("Query OK, number of affected rows: %1 in %2 s").arg(model->query().numRowsAffected() ).arg(nMS/1000.f);

        if (mToCSVcheckBox->isChecked() && model->rowCount()>0)
        {
            QFile lCsv(QDir::homePath()+"/GalaxySemi/temp/sqlbrowser.csv");
            if (lCsv.open(QIODevice::WriteOnly))
            {
                QSqlRecord	rec=model->record(0);
                if (mWriteLabelsInCVCheckBox->isChecked())
                {
                   for(int i=0; i<rec.count(); i++)
                   {
                        lCsv.write( rec.fieldName(i).toLatin1() );
                        lCsv.write( ", " );
                        if (i==rec.count()-1)
                            lCsv.write("\n");
                    }
                }
                for(int i=0; i<model->rowCount(); i++)
                {
                    rec=model->record(i);
                    for(int i=0; i<rec.count(); i++)
                    {
                        lCsv.write( rec.value(i).toString().toLatin1() );
                        lCsv.write( ", " );
                        if (i==rec.count()-1)
                            lCsv.write("\n");
                    }
                }

                lCsv.close();
            }
        }
    }
    mQueryGroupBox->setTitle(m);

    GSLOG(5, m.toLatin1().data());

    emit statusMessage(m);
    updateActions();
    table->resizeColumnsToContents(); // not too heavy ?
}

QSqlError SqlBrowserPrivate::addConnection(
        const QString &driver,
        const QString &dbName,
        const QString &host,
        const QString &user,
        const QString &passwd,
        int port)
{
    static int cCount = 0;

    QSqlError err;
    QSqlDatabase db = QSqlDatabase::addDatabase(driver, QString("Browser%1").arg(++cCount));
        db.setDatabaseName(dbName);
        db.setHostName(host);
        db.setPort(port);
        if (!db.open(user, passwd))
        {
            err = db.lastError();
            db = QSqlDatabase();
            QSqlDatabase::removeDatabase(QString("Browser%1").arg(cCount));
        }
    connectionWidget->refresh();

    return err;
}

void SqlBrowserPrivate::addConnection()
{
    QSqlConnectionDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    if (dialog.useInMemoryDatabase())
    {
        QSqlDatabase::database("in_mem_db", false).close();
        QSqlDatabase::removeDatabase("in_mem_db");
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "in_mem_db");
        db.setDatabaseName(":memory:");
        if (!db.open())
            QMessageBox::warning(this, tr("Unable to open database"), tr("An error occurred while "
                                                                         "opening the connection: ") + db.lastError().text());
        QSqlQuery q("", db);
        q.exec("drop table Movies");
        q.exec("drop table Names");
        q.exec("create table Movies (id integer primary key, Title varchar, Director varchar, Rating number)");
        q.exec("insert into Movies values (0, 'Metropolis', 'Fritz Lang', '8.4')");
        q.exec("insert into Movies values (1, 'Nosferatu, eine Symphonie des Grauens', 'F.W. Murnau', '8.1')");
        q.exec("insert into Movies values (2, 'Bis ans Ende der Welt', 'Wim Wenders', '6.5')");
        q.exec("insert into Movies values (3, 'Hardware', 'Richard Stanley', '5.2')");
        q.exec("insert into Movies values (4, 'Mitchell', 'Andrew V. McLaglen', '2.1')");
        q.exec("create table Names (id integer primary key, Firstname varchar, Lastname varchar, City varchar)");
        q.exec("insert into Names values (0, 'Sala', 'Palmer', 'Morristown')");
        q.exec("insert into Names values (1, 'Christopher', 'Walker', 'Morristown')");
        q.exec("insert into Names values (2, 'Donald', 'Duck', 'Andeby')");
        q.exec("insert into Names values (3, 'Buck', 'Rogers', 'Paris')");
        q.exec("insert into Names values (4, 'Sherlock', 'Holmes', 'London')");
        connectionWidget->refresh();
    }
    else
    {
        QSqlError err = addConnection(dialog.driverName(), dialog.databaseName(), dialog.hostName(),
                           dialog.userName(), dialog.password(), dialog.port());
        if (err.type() != QSqlError::NoError)
            QMessageBox::warning(this, tr("Unable to open database"), tr("An error occurred while "
                                       "opening the connection: ") + err.text());
    }
}

void SqlBrowserPrivate::showTable(const QString &t)
{
    GSLOG(5, QString("Show table %1").arg(t).toLatin1().data() );

    QSqlTableModel *model = new QSqlTableModel(table, connectionWidget->currentDatabase());
    model->setEditStrategy(QSqlTableModel::OnRowChange);
    model->setTable(connectionWidget->currentDatabase().driver()->escapeIdentifier(t, QSqlDriver::TableName));
    model->select();
    if (model->lastError().type() != QSqlError::NoError)
        emit statusMessage(model->lastError().text());
    table->setModel(model);
    table->setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed);

    connect(table->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            this, SLOT(currentChanged()));
    updateActions();
}

void SqlBrowserPrivate::showMetaData(const QString &t)
{

    GSLOG(5, QString("show Meta Data on %1").arg(t).toLatin1().data());

    QSqlRecord rec = connectionWidget->currentDatabase().record(t);
    QStandardItemModel *model = new QStandardItemModel(table);

    model->insertRows(0, rec.count());
    model->insertColumns(0, 7);

    model->setHeaderData(0, Qt::Horizontal, "Fieldname");
    model->setHeaderData(1, Qt::Horizontal, "Type");
    model->setHeaderData(2, Qt::Horizontal, "Length");
    model->setHeaderData(3, Qt::Horizontal, "Precision");
    model->setHeaderData(4, Qt::Horizontal, "Required");
    model->setHeaderData(5, Qt::Horizontal, "AutoValue");
    model->setHeaderData(6, Qt::Horizontal, "DefaultValue");


    for (int i = 0; i < rec.count(); ++i)
    {
        QSqlField fld = rec.field(i);
        model->setData(model->index(i, 0), fld.name());
        model->setData(model->index(i, 1), fld.typeID() == -1
                ? QString(QVariant::typeToName(fld.type()))
                : QString("%1 (%2)").arg(QVariant::typeToName(fld.type())).arg(fld.typeID()));
        model->setData(model->index(i, 2), fld.length());
        model->setData(model->index(i, 3), fld.precision());
        model->setData(model->index(i, 4), fld.requiredStatus() == -1 ? QVariant("?")
                : QVariant(bool(fld.requiredStatus())));
        model->setData(model->index(i, 5), fld.isAutoValue());
        model->setData(model->index(i, 6), fld.defaultValue());
    }

    table->setModel(model);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    updateActions();
}

void SqlBrowserPrivate::insertRow()
{
    QSqlTableModel *model = qobject_cast<QSqlTableModel *>(table->model());
    if (!model)
        return;

    QModelIndex insertIndex = table->currentIndex();
    int row = insertIndex.row() == -1 ? 0 : insertIndex.row();
    model->insertRow(row);
    insertIndex = model->index(row, 0);
    table->setCurrentIndex(insertIndex);
    table->edit(insertIndex);
}

void SqlBrowserPrivate::deleteRow()
{
    QSqlTableModel *model = qobject_cast<QSqlTableModel *>(table->model());
    if (!model)
        return;

    model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    QModelIndexList currentSelection = table->selectionModel()->selectedIndexes();
    for (int i = 0; i < currentSelection.count(); ++i) {
        if (currentSelection.at(i).column() != 0)
            continue;
        model->removeRow(currentSelection.at(i).row());
    }

    model->submitAll();
    model->setEditStrategy(QSqlTableModel::OnRowChange);

    updateActions();
}

void SqlBrowserPrivate::on_copyAction_triggered()
{
    QModelIndexList mil=table->selectionModel()->selectedIndexes(); // protected
    QString b;
    if (mil.size()>1)
    {
        foreach (QModelIndex mi, mil)
            b.append(mi.data().toString()+", ");
    }
    else
        b=table->currentIndex().data().toString();

    QApplication::clipboard()->setText( b );
}

void SqlBrowserPrivate::on_save_sqltextedit()
{
    GSLOG(5, "SqlBrowserPrivate::on_save_textedit");

    QFile f(QDir::homePath()+"/GalaxySemi/temp/sqlbrowser.txt");
    if (f.open(QIODevice::WriteOnly))
    {
        f.write(sqlEdit->toPlainText().toLatin1().data());
        f.close();
    }
}

void SqlBrowserPrivate::on_connectionWidget_tableActivated(const QString &table)
{
    GSLOG(5, QString("SqlBrowserPrivate::on_connectionWidget_tableActivated %1").arg(table).toLatin1().data());
    showTable(table);
}

void SqlBrowserPrivate::currentChanged()
{
    GSLOG(6, "SqlBrowserPrivate::currentChanged");
    updateActions();
}

void SqlBrowserPrivate::on_clearButton_clicked()
{
    sqlEdit->clear();
    sqlEdit->setFocus();
}

void SqlBrowserPrivate::on_submitButton_clicked()
{
    submitButton->setEnabled(false);
    clearButton->setEnabled(false);

    GSLOG(6, "SqlBrowserPrivate::on_submitButton_clicked()");

    exec();
    sqlEdit->setFocus();
    submitButton->setEnabled(true);
    clearButton->setEnabled(true);
}

void SqlBrowserPrivate::refreshDBlist()
{
    GSLOG(6, "SqlBrowserPrivate::refreshDBlist");
    connectionWidget->refresh();
}

void SqlBrowserPrivate::updateActions()
{
    GSLOG(6, "SqlBrowserPrivate::updateActions");

    bool enableIns = qobject_cast<QSqlTableModel *>(table->model());
    bool enableDel = enableIns && table->currentIndex().isValid();
    insertRowAction->setEnabled(enableIns);
    deleteRowAction->setEnabled(enableDel);
}

void SqlBrowserPrivate::about()
{
    QMessageBox::about(this, tr("About"), tr("The SQL Browser tool "
        "is NOT officially supported by Quantix. Use it (or not) at your own risk."
        "WT, Quantix dev team."));
}
} // QtLib
} // GS
