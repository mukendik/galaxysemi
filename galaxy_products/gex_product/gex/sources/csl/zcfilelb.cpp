#include <QDir>
#include <QTemporaryFile>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#include "zexcept.hpp"
#include "zfile.hpp"

#if ZC_WIN || ZC_OS2
  #include <direct.h>
  #include <sys\types.h>
  #include <sys\stat.h>
  #include <io.h>
#else
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/stat.h>
#endif


#include <sstream>

#ifdef ZC_NATIVECSLLIB
  #include "zcsl.hpp"
#else
  #include "zcslWrap.hpp"
#endif

#include "qstring.h"
#include "qstringlist.h"
#include "qdir.h"

using namespace std;

// Used for findFirst / findNext functions
static QStringList strListOfFiles;
static QStringList::Iterator itFile;
static QString strFileFolder;

class Stream : public ZBase
{
   public:
      Stream(istream* aistr, ostream* aostr, ZBoolean asys = zFalse):
         istr(aistr),
         ostr(aostr),
         sys(asys)
      {} // Stream

      Stream(const Stream& aStream):
         istr(aStream.istr),
         ostr(aStream.ostr),
         sys(aStream.sys)
      {} // Stream

      ZBoolean operator==(const Stream& aStream) const
      {
         return &aStream == this;
      } // operator ==

      ~Stream()
      {
         if (!sys) {
            if (istr) delete istr;
            else
               if (ostr) delete ostr;
         } // if
      } // ~Stream

      istream*		istr;             // input stream
      ostream*		ostr;             // output stream
      ZBoolean		sys;              // system stream
}; // Stream

class Streamlist : public ZBase
{
   public:
      Streamlist() {}
      ~Streamlist()
        { iList.drop(); }

      Streamlist(const Stream& aStream)
         { iList.addAsLast(new Stream(aStream)); }

      Streamlist& operator=(const Streamlist& aList)
         { iList = aList.iList; return *this; }

      Streamlist& operator+=(const Streamlist& aList)
         { iList += aList.iList; return *this; }

      Streamlist& operator+=(const Stream& aStream)
         { iList.addAsLast(new Stream(aStream)); return *this; }

      Streamlist& operator+=(Stream* aStream)
         { iList.addAsLast(aStream); return *this; }

      Stream& operator[](long aIndex) const
         { return *(Stream*)iList[aIndex]; }

      Streamlist& removeFromPos(long aIndex)
         { iList.removeFromPos(aIndex); return *this; }

      Streamlist& addAtPos(long aIndex, const Stream& aStream)
         { iList.addAtPos(aIndex, new Stream(aStream)); return *this; }

      Streamlist& addAtPos(long aIndex, Stream* aStream)
         { iList.addAtPos(aIndex, aStream); return *this; }

      Streamlist& addAsFirst(const Stream& aStream)
         { iList.addAsFirst(new Stream(aStream)); return *this; }

      Streamlist& addAsFirst(Stream* aStream)
         { iList.addAsFirst(aStream); return *this; }

      Streamlist& addAsLast(const Stream& aStream)
         { iList.addAsLast(new Stream(aStream)); return *this; }

      Streamlist& addAsLast(Stream* aStream)
         { iList.addAsLast(aStream); return *this; }

      Streamlist& drop()
         { iList.drop(); return *this; }

      long count() const
         { return iList.count(); }

      long find(const Stream& aStream) const
         { return iList.find(&aStream); }

   private:
      class List : public ZPointerlist
      {
         private:
            void* copy(void* aPointer) const
               { return new Stream(*(Stream*)(aPointer)); }

            void del(void* aPointer)
               { delete (Stream*)aPointer; }

            ZBoolean equal(const void* aPointer1, const void* aPointer2) const
               { return *(Stream*)aPointer1 == *(Stream*)aPointer2 ; }
      };

      List iList;
};

static Streamlist* iStrList = 0;
static long iUseCount(0);

Stream* getStream(ZCsl* csl)
{
   int handle = csl->get("handle").asInt();
   if (handle < 0 || handle >= iStrList->count())
      ZTHROWEXC("invalid file handle ("+ZString(handle)+")");
   Stream* s = &(*iStrList)[handle];
   if (!s->istr && !s->ostr)
      ZTHROWEXC("accessing closed file handle ("+ZString(handle)+")");
   return s;
} // getStream

static ZString fileOpen(ZCsl* csl)
{
   ZString fname = csl->get("filename");
   int     mode  = csl->get("mode").asInt();
#if defined unix
   iostream* str = new fstream(fname.constBuffer(), (std::_Ios_Openmode)mode);
#else
   iostream* str = new fstream(fname.constBuffer(), std::ios::openmode(mode));
#endif
   if (!str->good()) {
      delete str;
      ZTHROWEXC("error opening file "+fname);
   } // if
   iostream* istr = 0;
   iostream* ostr = 0;
   if (mode & (int)ios::in) istr = str;
   if (mode & (int)ios::out) ostr = str;
   iStrList->addAsLast(new Stream(istr, ostr));
   return iStrList->count()-1;
} // fileOpen

static ZString fileClose(ZCsl* csl)
{
   Stream* s = getStream(csl);
   if (s->sys)
      ZTHROWEXC("atemp to close system file");
   if (s->istr)
      delete s->istr;
   else
      delete s->ostr;
   s->istr = 0;
   s->ostr = 0;
   return 1;
} // fileClose

static ZString fileWrite(ZCsl* csl)
{
   Stream* s = getStream(csl);
   if (!s->ostr)
      ZTHROWEXC("write to readonly file");
   ZString data(csl->get("data"));
   s->ostr->write((char*)data, data.length());
   return 1;
} // fileWrite

static ZString fileWriteLine(ZCsl* csl)
{
   Stream* s = getStream(csl);
   if (!s->ostr)
      ZTHROWEXC("write to readonly file");
   if (csl->get("argCount").asInt() > 1) *s->ostr << csl->get("data");
   *s->ostr << endl;
   return 1;
} // fileWriteLine

static ZString fileWritePos(ZCsl* csl)
{
   Stream* s = getStream(csl);
   if (!s->ostr)
      ZTHROWEXC("write pos on readonly file");
   if (csl->get("argCount").asInt() > 1)
      s->ostr->seekp(csl->get("newpos").asInt());
   return (long)s->ostr->tellp();
} // fileWritePos

static ZString fileEof(ZCsl* csl)
{
   Stream* s = getStream(csl);
   if (!s->istr)
      ZTHROWEXC("eof from writeonly file");
   return (int)s->istr->eof();
} // fileEof


static ZString fileExists(ZCsl* csl)
{
   ZString fname = csl->get("filename");
    const QString strFileName = fname.constBuffer();
   if(QFile::exists(strFileName))
       return 1;	// File exists
   else
       return 0;	// file doesn't exist.
} // fileExists

static ZString fileFlush(ZCsl* csl)
{
   Stream* s = getStream(csl);
   if (!s->ostr)
      ZTHROWEXC("flush on readonly file");
   s->ostr->flush();
   return 1;
} // fileFlush

static ZString fileRead(ZCsl* csl)
{
   Stream* s = getStream(csl);
   if (!s->istr)
      ZTHROWEXC("read from writeonly file");
   int cnt(1);
   if (csl->get("argCount").asInt() > 1) cnt = csl->get("count").asInt();
   char* buf = new char[cnt+1];
   s->istr->read(buf, cnt);
   ZString ret(buf, s->istr->gcount());
   delete [] buf;
   return ret;
} // fileRead

static ZString fileReadLine(ZCsl* csl)
{
   Stream* s = getStream(csl);
   if (!s->istr)
      ZTHROWEXC("read from writeonly file");
   return ZString::lineFrom(*s->istr);
} // fileReadLine

static ZString fileReadPos(ZCsl* csl)
{
   Stream* s = getStream(csl);
   if (!s->istr)
      ZTHROWEXC("read pos on writeonly file");
   if (csl->get("argCount").asInt() > 1)
      s->istr->seekg(csl->get("newpos").asInt());
   return (long)s->istr->tellg();
} // fileReadPos

static ZString fileLocate(ZCsl* csl)
{
   return ZFile::locateFile(csl->get("filename"), csl->get("varname"));
} // fileLocate

static ZString convTime(time_t* tt)
{
   struct tm *t;
   t = localtime(tt);
   char buf[22];
   sprintf(buf,"%04d%02d%02d%02d%02d%02d",
           1900+t->tm_year, t->tm_mon+1, t->tm_mday,
           t->tm_hour, t->tm_min, t->tm_sec);
   ZString str;
   str = buf;
   return str;
} // convTime

static ZString fileInfo(ZCsl* csl)
{
   ZString file(csl->get("filename"));
   ZString path(ZFile::fullPath(file));
   struct stat buf;
   if (stat(path.constBuffer(), &buf))
       ZTHROWEXC("file could not be found ("+file+")");
   return
       ZString((unsigned long)buf.st_size)+" "+
       convTime(&buf.st_atime)+" "+
       convTime(&buf.st_mtime)+" "+
       convTime(&buf.st_ctime)+" "+
       path;
} // fileInfo

static ZString fileDelDir(ZCsl* csl)
{
   ZString name(csl->get("dirname"));
#if ZC_GNU
   if (rmdir(name.constBuffer()))
#else
   if (rmdir(name))
#endif
      ZTHROWEXC("error deleting directory ("+name+")");
   return 1;
} // fileDelDir

static ZString fileMakeDir(ZCsl* csl)
{
   ZString name(csl->get("dirname"));
#if ZC_WIN || ZC_OS2
 #if ZC_GNU
   if (mkdir(name.constBuffer()))
 #else
   if (mkdir(name))
 #endif
#else
   if (mkdir(name.constBuffer(),0755))
#endif
      ZTHROWEXC("error creating directory ("+name+")");
   return 1;
} // fileMakeDir

static ZString fileDelete(ZCsl* csl)
{
   ZString name(csl->get("filename"));
   // Change file mode in case it is read-only
#if defined unix || __MACH__
        chmod(name,0777);
#else
        _chmod(name,_S_IREAD | _S_IWRITE);
#endif

    // If file doesn't exists, quietly return!
        if(QFile::exists(name.constBuffer()) == false)
            return 1;
    // file ecists, then remove it!
   if (remove(name.constBuffer()))
      ZTHROWEXC("error deleting file ("+name+")");
   return 1;
} // fileDelete

static ZString fileRename(ZCsl* csl)
{
   ZString oldn(csl->get("oldname"));
   ZString newn(csl->get("newname"));
   if (rename(oldn.constBuffer(),newn.constBuffer()))
      ZTHROWEXC("error renaming file ("+oldn+" -> "+newn+")");
   return 1;
} // fileRename

static ZString fileTempName(ZCsl* /*csl*/)
{
    // tempnam() seems no more supported by gcc 4.5+
    // Use safer mkstemp ?
    //char* name = mkstemp("XXXXXX.Csl");
    // or let s use Qt
    QTemporaryFile lTmpFile("XXXXXX.Csl");
    if (!lTmpFile.open())
        ZTHROWEXC("error creating temp filename");
    char* name=lTmpFile.fileName().toLatin1().data();
    lTmpFile.close();

   if (!*name)
      ZTHROWEXC("error creating temp filename");
   ZString ret(name);
   free(name);
   return ret;
} // fileTempName

static ZString fileCopy(ZCsl* csl)
{
//   fstream src(csl->get("source").constBuffer(), ios::in | ios::nocreate | ios::binary);
   fstream src(csl->get("source").constBuffer(), ios::in | ios::binary);
   if (!src.good())
      ZTHROWEXC("error opening source file ("+csl->get("source")+")");
   fstream dst(csl->get("dest").constBuffer(), ios::out | ios::trunc | ios::binary);
   if (!dst.good())
      ZTHROWEXC("error opening destination file ("+csl->get("dest")+")");
   char* buf = new char[512];
   while (!src.eof()) {
      src.read(buf, 512);
      dst.write(buf, src.gcount());
   } // while
   delete [] buf;
   return 1;
} // fileCopy

// Find all files in a given folder.
static ZString fileFindFirst(ZCsl* csl)
{
    ZString fdirname = csl->get("dirname");	// Folder to scan
    ZString fext = csl->get("ext");			// Extensions to look for. eg: "*.std;*.atd"

    QDir d;
    QString strString;
    strFileFolder = fdirname;
    d.setPath(strFileFolder);
    d.setFilter(QDir::Files);
    strString = fext;
    strListOfFiles = d.entryList(QStringList(strString));
    itFile = strListOfFiles.begin();

    if(strListOfFiles.count() > 0)
    {
        QString strFilePath;
        ZString ZstrFilePath;
        strFilePath = fdirname;
        if(strFilePath.endsWith("/") == false)
            strFilePath+= "/";
        strFilePath += *itFile;
        itFile++;	// Move to next file, ready for next reading.
        ZstrFilePath = strFilePath.toLatin1().constData();
        return ZstrFilePath;
    }

   return "";	// Return empty string if no file found.
} // fileFindFirst

// Find all files in a given folder.
static ZString fileFindNext(ZCsl* /*csl*/)
{
    if(itFile == strListOfFiles.end())
        return "";	// All files read.

    QString strFilePath = strFileFolder;
    if(strFilePath.endsWith("/") == false)
        strFilePath+= "/";
    strFilePath += *itFile;
    itFile++;	// Move to next file, ready for next reading.

    ZString ZstrFilePath;
    ZstrFilePath = strFilePath.toLatin1().constData();

    return ZstrFilePath;
} // fileFindNext

void ZCslInitLib_File(ZCsl* aCsl)
{
   if (iUseCount++ == 0 && !iStrList)
      iStrList = new Streamlist;

   if (!iStrList->count())
      (*iStrList)
         .addAsLast(new Stream(&cin,  0, zTrue))
         .addAsLast(new Stream(0, &cout, zTrue))
         .addAsLast(new Stream(0, &cerr, zTrue));

   ZString iFile("FileLib");

   ZString init(ZString(
      "const fileVersion = '3.00';\n"
      "const fileCompiler = '")+ZC_COMPILER+"';\n"
      "const fileLibtype = '"+ZC_CSLLIBTYPE+"';\n"
      "const fileBuilt = '"+ZString(__DATE__)+" "+__TIME__+"';\n"

      "const fileStdIn = 0;\n"
      "const fileStdOut = 1;\n"
      "const fileStdErr = 2;\n"

      "const fileOpenRead = "+ZString((int)ios::in)+";\n"
      "const fileOpenWrite = "+ZString((int)ios::out)+";\n"
      "const fileOpenAppend = "+ZString((int)ios::app)+";\n"
      "const fileOpenTrunc = "+ZString((int)ios::trunc)+";\n"
//      "const fileOpenOld = "+ZString((int)ios::nocreate)+";\n"
//      "const fileOpenNew = "+ZString((int)ios::noreplace)+";\n"
      "const fileOpenBinary = "+ZString((int)ios::binary)+";\n"
   );
   istringstream str((char*)init);
   aCsl->loadScript(iFile, &str);

   (*aCsl)
      .addFunc(iFile,
         "fileClose("
            "const handle)",
          fileClose)

      .addFunc(iFile,
         "fileCopy("
            "const source, "
            "const dest)",
          fileCopy)

      .addFunc(iFile,
         "fileDelete("
            "const filename)",
          fileDelete)

      .addFunc(iFile,
         "fileDelDir("
            "const dirname)",
          fileDelDir)

      .addFunc(iFile,
         "fileEof("
            "const handle)",
          fileEof)

      .addFunc(iFile,
         "fileExists("
            "const filename)",
          fileExists)

      .addFunc(iFile,
         "fileFindFirst("
            "const dirname, "
            "const ext)",
          fileFindFirst)

      .addFunc(iFile,
         "fileFindNext()",
          fileFindNext)

          .addFunc(iFile,
         "fileFlush("
            "const handle)",
          fileFlush)

      .addFunc(iFile,
         "fileInfo("
            "const filename)",
          fileInfo)

      .addFunc(iFile,
         "fileLocate("
            "const filename, "
            "const varname)",
          fileLocate)

      .addFunc(iFile,
         "fileMakeDir("
            "const dirname)",
          fileMakeDir)

      .addFunc(iFile,
         "fileOpen("
            "const filename, "
            "const mode)",
          fileOpen)

      .addFunc(iFile,
         "fileRead("
            "const handle, "
            "[const count])",
          fileRead)

      .addFunc(iFile,
         "fileReadLine("
            "const handle)",
          fileReadLine)

      .addFunc(iFile,
         "fileReadPos("
            "const handle, "
            "[const newpos])",
          fileReadPos)

      .addFunc(iFile,
         "fileRename("
            "const oldname, "
            "const newname)",
          fileRename)

      .addFunc(iFile,
         "fileTempName()",
          fileTempName)

      .addFunc(iFile,
         "fileWrite("
            "const handle, "
            "const data)",
          fileWrite)

      .addFunc(iFile,
         "fileWriteLine("
            "const handle, "
            "[const data])",
          fileWriteLine)

      .addFunc(iFile,
         "fileWritePos("
            "const handle, "
            "[const newpos])",
          fileWritePos);
} // ZCslInitLib

void ZCslCleanupLib_File(ZCsl* /*aCsl*/)
{
   if (--iUseCount == 0 && iStrList)
   {
      delete iStrList;
      iStrList = 0;
   } // if
} // ZCslCleanupLib
