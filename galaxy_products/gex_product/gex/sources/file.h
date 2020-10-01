#ifndef FILE_H
#define FILE_H

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueList>
#include "gex_scriptengine.h"

extern GexScriptEngine*	pGexScriptEngine;

/*
  This File class is a generic class to play with files.
  Advantages over QFile is that this one is accessible through JavaScript
*/
class File : public QFile
{
  Q_OBJECT

    //QList<QString> m;

public slots:

  /*
  QScriptValue CopyFilesToFolder(int s)
  {
      QScriptValue sv=pGexScriptEngine->newArray( s );
      for (int i=0; i<s; i++)
          sv.setProperty(i, rand());
      //sv.setProperty(1, 654);
      //sv.setProperty(2, 4);
      //sv=QScriptValue("hello");
      //QScriptValueList svl;
      //sv.setData(svl);
      //svl.append(s);
      //sv=svl.to
      //svl.append(2);
      return sv;
  }
  */

QScriptValue List(const QString folder, const QString regexp="")
{
    QDir lDir(folder);
    if (!lDir.exists())
        return QScriptValue();

    QFileInfoList lFIL=lDir.entryInfoList();
    QScriptValue sv=pGexScriptEngine->newArray( ); // lFIL.size()
    QRegExp lRE(regexp);
    int i=0;
    foreach(const QFileInfo &fi, lFIL)
    {
        //pGexScriptEngine->newQObject( ?
        if (!regexp.isEmpty())
        {
            if (lRE.indexIn(fi.fileName())==-1)
                continue;
        }
        sv.setProperty(i++, fi.fileName());
    }
    return sv;
  }

  QString Remove(const QString& filename)
  {
      QFile lFile(filename);
      if (lFile.remove())
          return "ok";
      return "error : cant remove file";
  }

  QString Copy(QString source, QString target)
  {
    if (!QFile::exists(source))
      return "error : source file unfindable";
    if (QFile::exists(target))
      QFile::remove(target); //QDir::rmpath()
    bool b=QFile::copy(source, target);
    return b?"ok":("error : cannot copy "+source+" to "+target);
  }

  bool Exist(QString f)
  {
    return QFile::exists(f);
  }

  QString WriteAsText(const QString &lOutputFilename, const QString &lContent)
  {
      QFile f(lOutputFilename);
      if (!f.open(QIODevice::WriteOnly))
        return "error : cannot write open file " + lOutputFilename;
      qint64 lRes=f.write(lContent.toLatin1());
      return "ok: "+QString::number(lRes)+"bytes written";
  }

  QString ReadAsText(QString lFilename)
  {
      if (lFilename.isEmpty())
          return "error: filename empty";
      QFile f(lFilename);
      if (!f.open(QIODevice::ReadOnly))
        return "error : cannot read file " + lFilename;

      QTextStream ts(&f);
      QString lAll=ts.readAll();
      f.close();
      return lAll;
  }

  QString SearchString(QString filename, QString key)
  {
      QFile f(filename);
      if (!f.open(QIODevice::ReadOnly))
        return "error : cant read file " + filename;

      QTextStream ts(&f);
      QString s=ts.readAll();
      if (s.isEmpty())
      {
          f.close();
          return "not found";
      }

      int lR=s.indexOf(key);

      f.close();

      if (lR==-1)
          return "not found";

      return "found";
  }

  QString ReplaceString(QString file, QString from, QString to)
  {
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly))
      return "error : cant read file " + file;

    QTextStream ts(&f);
    QString s=ts.readAll();
    if (s.isEmpty())
    {
        f.close();
        return "ok (file empty)";
    }
    s.replace(from, to);
    //ts.seek(0);
    //f.reset();
    f.close();
    if (!f.open(QIODevice::WriteOnly))
      return "error : cant write into file " + file;
    f.write(s.toLatin1().data());
    f.close();
    return "ok";
  }

  bool MkPath(const QString &lPath)
  {
      QDir lDir(lPath);
      return lDir.mkpath(lPath);
  }

};

#endif // FILE_H
