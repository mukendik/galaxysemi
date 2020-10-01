#ifndef EXPORT_CSV_H
#define EXPORT_CSV_H

#include <QString>
class QProgressDialog;
class AbstractCsvConverter;
class QDate;

class CSTDFtoCSV
{
public:

    CSTDFtoCSV();
    ~CSTDFtoCSV();

    bool                        Convert(const QString& stdfFileName, const QString& csvFileName);
    QString                     GetLastError() const;

private:

    QString                     mCsvVersion;
    AbstractCsvConverter *      mPrivateCSVConverter;
public:
    void setProgressDialog(QProgressDialog* poProgDialog, int iFileNumber, int iFileCount);
};

#endif // EXPORT_CSV_H
