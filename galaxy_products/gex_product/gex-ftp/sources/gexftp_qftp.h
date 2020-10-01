#ifndef GEXFTP_QFTP_H
#define GEXFTP_QFTP_H

#include <QtCore/qstring.h>
#include "qurlinfo.h"
#include <QtCore/qobject.h>

// Added by Galaxy
#include <QTcpSocket>
#include <QTcpServer>
#include <QTimer>
#include <QStringList>

//QT_BEGIN_HEADER

//QT_BEGIN_NAMESPACE

//QT_MODULE(Network)

#ifndef QT_NO_FTP

class GexFtpQFtpPrivate;

class /*Q_NETWORK_EXPORT*/ GexFtpQFtp : public QObject
{
    Q_OBJECT

public:
    explicit GexFtpQFtp(QObject *parent = 0);
    virtual ~GexFtpQFtp();

    enum State {
        Unconnected,
        HostLookup,
        Connecting,
        Connected,
        LoggedIn,
        Closing
    };
    enum Error {
        NoError,
        UnknownError,
        HostNotFound,
        ConnectionRefused,
        NotConnected
    };
    enum Command {
        None,
        SetTransferMode,
        SetProxy,
        ConnectToHost,
        Login,
        Close,
        List,
        Cd,
        Get,
        Put,
        Remove,
        Mkdir,
        Rmdir,
        Rename,
        RawCommand
    };
    enum TransferMode {
        Active,
        Passive
    };
    enum TransferType {
        Binary,
        Ascii
    };

    int setProxy(const QString &host, quint16 port);
    int connectToHost(const QString &host, quint16 port=21);
    int login(const QString &user = QString(), const QString &password = QString());
    int close();
    int setTransferMode(TransferMode mode);
    int list(const QString &dir = QString());
    int cd(const QString &dir);
    int get(const QString &file, QIODevice *dev=0, TransferType type = Binary);
    int put(const QByteArray &data, const QString &file, TransferType type = Binary);
    int put(QIODevice *dev, const QString &file, TransferType type = Binary);
    int remove(const QString &file);
    int mkdir(const QString &dir);
    int rmdir(const QString &dir);
    int rename(const QString &oldname, const QString &newname);

    int rawCommand(const QString &command);

    qint64 bytesAvailable() const;
    qint64 read(char *data, qint64 maxlen);
#ifdef QT3_SUPPORT
    inline QT3_SUPPORT qint64 readBlock(char *data, quint64 maxlen)
    { return read(data, qint64(maxlen)); }
#endif
    QByteArray readAll();

    int currentId() const;
    QIODevice* currentDevice() const;
    Command currentCommand() const;
    bool hasPendingCommands() const;
    void clearPendingCommands();

    State state() const;

    Error error() const;
    QString errorString() const;

public Q_SLOTS:
    void abort();

Q_SIGNALS:
    void stateChanged(int);
    void listInfo(const QUrlInfo&);
    void readyRead();
    void dataTransferProgress(qint64, qint64);
    void rawCommandReply(int, const QString&);

    void commandStarted(int);
    void commandFinished(int, bool);
    void done(bool);

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR GexFtpQFtp(QObject *parent, const char *name);
#endif

private:
    Q_DISABLE_COPY(GexFtpQFtp)
    Q_DECLARE_PRIVATE(GexFtpQFtp)

    Q_PRIVATE_SLOT(d_func(), void _q_startNextCommand())
    Q_PRIVATE_SLOT(d_func(), void _q_piFinished(const QString&))
    Q_PRIVATE_SLOT(d_func(), void _q_piError(int, const QString&))
    Q_PRIVATE_SLOT(d_func(), void _q_piConnectState(int))
    Q_PRIVATE_SLOT(d_func(), void _q_piFtpReply(int, const QString&))
};


class GexFtpQFtpPI;

/*
    The GexFtpQFtpDTP (DTP = Data Transfer Process) controls all client side
    data transfer between the client and server.
*/
class GexFtpQFtpDTP : public QObject
{
    Q_OBJECT

public:
    enum ConnectState {
        CsHostFound,
        CsConnected,
        CsClosed,
        CsHostNotFound,
        CsConnectionRefused
    };

    GexFtpQFtpDTP(GexFtpQFtpPI *p, QObject *parent = 0);

    void setData(QByteArray *);
    void setDevice(QIODevice *);
    void writeData();
    void setBytesTotal(qint64 bytes);

    bool hasError() const;
    QString errorMessage() const;
    void clearError();

    void connectToHost(const QString & host, quint16 port);
    int setupListener(const QHostAddress &address);
    void waitForConnection();

    QTcpSocket::SocketState state() const;
    qint64 bytesAvailable() const;
    qint64 read(char *data, qint64 maxlen);
    QByteArray readAll();

    void abortConnection();

    static bool parseDir(const QByteArray &buffer, const QString &userName, QUrlInfo *info);

signals:
    void listInfo(const QUrlInfo&);
    void readyRead();
    void dataTransferProgress(qint64, qint64);

    void connectState(int);

private slots:
    void socketConnected();
    void socketReadyRead();
    void socketError(QAbstractSocket::SocketError);
    void socketConnectionClosed();
    void socketBytesWritten(qint64);
    void setupSocket();

    void dataReadyRead();

private:
    void clearData();

    QTcpSocket *socket;
    QTcpServer listener;

    GexFtpQFtpPI *pi;
    QString err;
    qint64 bytesDone;
    qint64 bytesTotal;
    bool callWriteData;

    // If is_ba is true, ba is used; ba is never 0.
    // Otherwise dev is used; dev can be 0 or not.
    union {
        QByteArray *ba;
        QIODevice *dev;
    } data;
    bool is_ba;

    QByteArray bytesFromSocket;
};

/**********************************************************************
 *
 * GexFtpQFtpPI - Protocol Interpreter
 *
 *********************************************************************/

class GexFtpQFtpPI : public QObject
{
    Q_OBJECT

public:
    GexFtpQFtpPI(QObject *parent = 0);

    void connectToHost(const QString &host, quint16 port);

    bool sendCommands(const QStringList &cmds);
    bool sendCommand(const QString &cmd)
        { return sendCommands(QStringList(cmd)); }

    void clearPendingCommands();
    void abort();

    QString currentCommand() const
        { return currentCmd; }

    bool rawCommand;
    bool transferConnectionExtended;

    GexFtpQFtpDTP dtp; // the PI has a DTP which is not the design of RFC 959, but it
                 // makes the design simpler this way
signals:
    void connectState(int);
    void finished(const QString&);
    void error(int, const QString&);
    void rawFtpReply(int, const QString&);

private slots:
    void hostFound();
    void connected();
    void connectionClosed();
    void delayedCloseFinished();
    void readyRead();
    void error(QAbstractSocket::SocketError);

    void dtpConnectState(int);

private:
    // the states are modelled after the generalized state diagram of RFC 959,
    // page 58
    enum State {
        Begin,
        Idle,
        Waiting,
        Success,
        Failure
    };

    enum AbortState {
        None,
        AbortStarted,
        WaitForAbortToFinish
    };

    bool processReply();
    bool startNextCmd();

    QTcpSocket commandSocket;
    QString replyText;
    char replyCode[3];
    State state;
    AbortState abortState;
    QStringList pendingCommands;
    QString currentCmd;

    bool waitForDtpToConnect;
    bool waitForDtpToClose;

    QByteArray bytesFromSocket;

    friend class GexFtpQFtpDTP;
};

/**********************************************************************
 *
 * GexFtpQFtpCommand implemenatation
 *
 *********************************************************************/
class GexFtpQFtpCommand
{
public:
    GexFtpQFtpCommand(GexFtpQFtp::Command cmd, QStringList raw, const QByteArray &ba);
    GexFtpQFtpCommand(GexFtpQFtp::Command cmd, QStringList raw, QIODevice *dev = 0);
    ~GexFtpQFtpCommand();

    int id;
    GexFtpQFtp::Command command;
    QStringList rawCmds;

    // If is_ba is true, ba is used; ba is never 0.
    // Otherwise dev is used; dev can be 0 or not.
    union {
        QByteArray *ba;
        QIODevice *dev;
    } data;
    bool is_ba;

    static QBasicAtomicInt idCounter;
};

/**********************************************************************
 *
 * GexFtpQFtpPrivate
 *
 *********************************************************************/

QT_BEGIN_INCLUDE_NAMESPACE
#include <qtbase/src/corelib/kernel/qobject_p.h>
QT_END_INCLUDE_NAMESPACE

class GexFtpQFtpPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(GexFtpQFtp)
public:

    inline GexFtpQFtpPrivate() : close_waitForStateChange(false), state(GexFtpQFtp::Unconnected),
                           transferMode(GexFtpQFtp::Passive), error(GexFtpQFtp::NoError)
    { }

    ~GexFtpQFtpPrivate() { while (!pending.isEmpty()) delete pending.takeFirst(); }

    // private slots
    void _q_startNextCommand();
    void _q_piFinished(const QString&);
    void _q_piError(int, const QString&);
    void _q_piConnectState(int);
    void _q_piFtpReply(int, const QString&);

    int addCommand(GexFtpQFtpCommand *cmd);

    GexFtpQFtpPI pi;
    QList<GexFtpQFtpCommand *> pending;
    bool close_waitForStateChange;
    GexFtpQFtp::State state;
    GexFtpQFtp::TransferMode transferMode;
    GexFtpQFtp::Error error;
    QString errorString;

    QString host;
    quint16 port;
    QString proxyHost;
    quint16 proxyPort;
};

#endif // QT_NO_FTP

//QT_END_NAMESPACE

//QT_END_HEADER

#endif // GEXFTP_QFTP_H
