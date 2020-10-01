QuaZIP is the C++ wrapper for the Gilles Vollant's ZIP/UNZIP package
using Trolltech's Qt library.

It contains original ZIP/UNZIP package C code and therefore depends on
zlib library.

Also, it depends on Qt 4.

To compile it on UNIX dialect:

$ cd quazip
$ qmake
$ make

You must make sure that:
* You have Qt 4 properly and fully installed (including tools and
  headers, not just library)
* "qmake" command runs Qt 4's qmake, not some other version (you'll have
  to type full path to qmake otherwise).

To install compiled shared library, just type:

$ make install

By default, it installs in /usr/local, but you may change it using

$ qmake PREFIX=/wherever/you/whant/to/install

You do not have to compile and install QuaZIP to use it. You can just
(and sometimes it may be the best way) add QuaZIP's source files to your
project and use them.

See doc/html or, if you do not have a browser, quazip/*.h and
quazip/doc/* files for the more detailed documentation.

For Windows, it's essentially the same, but you may have to adjust
settings for different environments.

If you want to include QuaZIP sources directly into your project or if
you want to use QuaZIP compiled as a static library using
"qmake CONFIG+=statliclib", you have to define the QUAZIP_STATIC macro,
otherwise you're likely to run into problems as QuaZIP symbols will be
marked as dllimported.

Copyright notice:

Copyright (C) 2005-2011 Sergey A. Tachenov

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

See COPYING file for the full LGPL text.

Original ZIP package is copyrighted by Gilles Vollant, see
quazip/(un)zip.h files for details, basically it's zlib license.



Example :

#include "quazip/quazip.h"
#include "quazip/quazipfile.h"

static bool archive(const QString & filePath, const QDir & dir, const QString & comment = QString("")) 
{

	QuaZip zip(filePath);
	zip.setFileNameCodec("IBM866");

	if (!zip.open(QuaZip::mdCreate)) {
		myMessageOutput(true, QtDebugMsg, QString("testCreate(): zip.open(): %1").arg(zip.getZipError()));
		return false;
	}

	if (!dir.exists()) {
		myMessageOutput(true, QtDebugMsg, QString("dir.exists(%1)=FALSE").arg(dir.absolutePath()));
		return false;
	}

	QFile inFile;

	// ???????? ?????? ?????? ? ????? ??????????
	QStringList sl;
	recurseAddDir(dir, sl);

	// ??????? ?????? ????????? ?? QFileInfo ????????
	QFileInfoList files;
	foreach (QString fn, sl) files << QFileInfo(fn);

	QuaZipFile outFile(&zip);

	char c;
	foreach(QFileInfo fileInfo, files) {

		if (!fileInfo.isFile())
			continue;

		// ???? ???? ? ?????????????, ?? ????????? ??? ???? ????????????? ? ?????? ??????
		// ????????: fileInfo.filePath() = "D:\Work\Sources\SAGO\svn\sago\Release\tmp_DOCSWIN\Folder\123.opn"
		// ????? ????? ???????? ????? ?????? fileNameWithSubFolders ????? ????? "Folder\123.opn" ? ?.?.
		QString fileNameWithRelativePath = fileInfo.filePath().remove(0, dir.absolutePath().length() + 1);

		inFile.setFileName(fileInfo.filePath());

		if (!inFile.open(QIODevice::ReadOnly)) {
			myMessageOutput(true, QtDebugMsg, QString("testCreate(): inFile.open(): %1").arg(inFile.errorString().toLocal8Bit().constData()));
			return false;
		}

		if (!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileNameWithRelativePath, fileInfo.filePath()))) {
			myMessageOutput(true, QtDebugMsg, QString("testCreate(): outFile.open(): %1").arg(outFile.getZipError()));
			return false;
		}

		while (inFile.getChar(&c) && outFile.putChar(c));

		if (outFile.getZipError() != UNZ_OK) {
			myMessageOutput(true, QtDebugMsg, QString("testCreate(): outFile.putChar(): %1").arg(outFile.getZipError()));
			return false;
		}

		outFile.close();

		if (outFile.getZipError() != UNZ_OK) {
			myMessageOutput(true, QtDebugMsg, QString("testCreate(): outFile.close(): %1").arg(outFile.getZipError()));
			return false;
		}

		inFile.close();
	}

	// + ???????????
	if (!comment.isEmpty())
		zip.setComment(comment);

	zip.close();

	if (zip.getZipError() != 0) {
		myMessageOutput(true, QtDebugMsg, QString("testCreate(): zip.close(): %1").arg(zip.getZipError()));
		return false;
	}

	return true;
}


