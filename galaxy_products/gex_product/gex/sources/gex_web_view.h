#ifndef GEX_WEB_VIEW_H
#define GEX_WEB_VIEW_H

#ifdef GEX_NO_WEBKIT
#include <QTextBrowser>
#else
#include <QWebView>
#endif

// Name			:	GexWebView
//
// Description	:	Web viewer based on QWebView
//

class GexWebView :
#ifdef GEX_NO_WEBKIT
        public QTextBrowser
#else
        public QWebView
#endif
{
    Q_OBJECT

    bool mLoadFinished; // simply say if load finished. Reseted to false in onLoadStarted.
    QUrl mLastUrlClicked; // Updated in onLinkClicked

public:
    GexWebView(QWidget * pParent = NULL);
    ~GexWebView();

    const QUrl& GetLastUrlCliked() { return mLastUrlClicked; }

    // Overwrite virtual fcts from main class for Drag&Drop support.
    void	dragEnterEvent(QDragEnterEvent * pDragEnterEvent);
    void	dragMoveEvent(QDragMoveEvent * pDragMoveEvent);
    void	dropEvent(QDropEvent * pDropEvent);

    #ifndef GEX_NO_WEBKIT
        public slots:
        void onLoadProgress( int progress );
        void onLoadStarted( );
        void onLoadFinished( bool ok );
        void onStatusBarMessage( const QString & text );
        void onTitleChanged( const QString & title );
        void onUrlChanged( const QUrl & url );
        void onLinkClicked( const QUrl & url );
        bool loadFinished() { return mLoadFinished; }
    #endif
};

#endif // GEX_WEB_VIEW_H
