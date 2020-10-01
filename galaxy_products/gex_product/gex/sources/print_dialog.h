///////////////////////////////////////////////////////////
// All classes to manage the GEX Print dialog box.
///////////////////////////////////////////////////////////
#ifndef GEX_PRINT_H
#define GEX_PRINT_H

#include "ui_print_dialog.h"

#include <time.h>
#include <QtPrintSupport/QPrinter>
#include <QTextBrowser>
#include <QWebView>
#include "browser_dialog.h"
#include "scripting_io.h"
#include "gex_constants.h"

class GexPrint : public QDialog, public Ui::print_basedialog
{
	Q_OBJECT
		
public:
	GexPrint( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    //! \brief Destructor: should delete font
    ~GexPrint();
    //! \brief Print html pages using given printer
    void PrintPages(QPrinter *printer, QPainter *p, const QUrl& urlCurrent, QString strPrintReport);
    //! \brief Returns 'true' if it's not a HTML but a Widget to print!
    bool IsWidgetToPrint(void);

private:
	QFont *pFont;
    QTextBrowser* mHtmlReport;
    QWebView mWebView;
	int dpix;
	int dpiy;
	QRect bodyRef;
	QRect body;
    //! \brief ?
    int mPage;
	QString strFooter;
    //! \brief Print the given html file using given printer and painter
    QString PrintHtmlFile(QPrinter *printer, QPainter *p, QString file);
    //! \brief PRINT the content of a given Widget
    void PrintPixmap(QPrinter *printer, QPainter *p, QPixmap *pPixmap);
	void UpdateStatusMessage(QString strMessage);

public slots:
	void OnPrintCurrentPage(void);
	void OnPrintReportSections(void);
    //! \brief Updates fields: enable/disable depending of situation
    void UpdateView(QString strHtmlReport);
    //! \brief
    void OnLoadFinished(bool);
    //! \brief
    QString PrintHtmlToPDF(const QString &source, const QString &output);

};

#endif
