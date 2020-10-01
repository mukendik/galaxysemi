///////////////////////////////////////////////////////////
// All classes used for 'Assitant' (Neptus contxtext help)
///////////////////////////////////////////////////////////

#ifndef ASSISTANT_H
#define ASSISTANT_H
#include <qwidget.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qfile.h>
#include <qmenu.h> 
#include <qcursor.h> 
#include <stdlib.h>
#include <qstring.h> 
#include <qtextbrowser.h> 


class FlyingNeptus : public QWidget
{
public:
    
	FlyingNeptus( QWidget *parent=0, Qt::WindowFlags f = 0);
	
	void NotifyPageChange(QString strHtmlPage);
	void ShowNeptusHelpPixmap(bool);
	void loadShowPage(QString strHtmlPage);
	QTextBrowser *FlyingNeptusPage;

protected:

    void mousePressEvent( QMouseEvent *);
	void mouseReleaseEvent ( QMouseEvent * e );
    void mouseMoveEvent( QMouseEvent *);
	void focusOutEvent ( QFocusEvent * );

private:

	QPixmap *pixIdle;
	QPixmap *pixHelp;
    QPoint clickPos;
	bool	bMoving;
	bool	bNeptusHelpPixmap;
	bool	bSmartNeptusClose;
	bool	bUserNeptusClose;
};
#endif
