// This is from
// http://lau.engineering.uky.edu/2013/08/17/qt-based-tftp-class/
// Seems to implement Trivial FTP

#define MAXPACKETSIZE 512

#include <QDebug>
#include <QWidget>
#include <QTextEdit>
#include <QProcess>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QCoreApplication>
#include <QNetworkInterface>
#include <QList>
#include <QLabel>
#include <QUdpSocket>


class QTFTPWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QTFTPWidget(QString localAddressString = QString(), int localPort = -1,
                           QString remoteAddressString = QString(), int remotePort = -1,
                           QWidget *parent = 0);
    ~QTFTPWidget();

    bool putByteArray(QString filename, QByteArray transmittingFile);
    bool getByteArray(QString filename, QByteArray *requestedFile);

    inline QString errorString() { return(lastErrorString); }

    QString remoteAddressString();
    QString localAddressString();
    int remotePortNumber();
    int localPortNumber();

public slots:
    inline void onSetLocalAddress(QString address) { localAddressSuppliedByUser=address; }
    inline void onSetLocalPort(int port) { localPortSuppliedByUser=port; }
    inline void onSetRemoteAddress(QString address) { remoteAddressSuppliedByUser=address; }
    inline void onSetRemotePort(int port) { remotePortSuppliedByUser=port; }

private:
    QString localAddressSuppliedByUser, remoteAddressSuppliedByUser;
    int localPortSuppliedByUser, remotePortSuppliedByUser;

    QString lastErrorString;
    QUdpSocket *socket;
    QComboBox *localAddressComboBox;
    QSpinBox *localPortSpinBox;
    QLineEdit *remoteAddressLineEdit;
    QSpinBox *remotePortSpinBox;

    bool bindSocket();
    QByteArray getFilePacket(QString filename);
    QByteArray putFilePacket(QString filename);
};

QTFTPWidget::QTFTPWidget(QString localAddressString, int localPort, QString remoteAddressString, int remotePort, QWidget *parent) : QWidget(parent),
                             localAddressSuppliedByUser(localAddressString), localPortSuppliedByUser(localPort),
                             remoteAddressSuppliedByUser(remoteAddressString), remotePortSuppliedByUser(remotePort)
{
    this->setWindowTitle(QString("TFTP Widget"));
    this->setLayout(new QVBoxLayout());
    this->layout()->setSpacing(1);

    QWidget *widget=new QWidget();
    widget->setLayout(new QHBoxLayout());
    widget->layout()->setContentsMargins(0, 0, 0, 0);

    localAddressComboBox = new QComboBox();
    localAddressComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    if (localAddressSuppliedByUser.isEmpty()){
        QList<QHostAddress> hstList = QNetworkInterface::allAddresses();
        for (int n=0; n<hstList.count(); n++){
            if (hstList.at(n).toIPv4Address()){
                QString string=hstList.at(n).toString();
                localAddressComboBox->addItem(string);
            }
        }
    } else {
        localAddressComboBox->addItem(localAddressSuppliedByUser);
        localAddressComboBox->setDisabled(true);
    }

    localPortSpinBox = new QSpinBox();
    localPortSpinBox->setFixedWidth(100);
    localPortSpinBox->setMaximum(65535);
    if (localPortSuppliedByUser > 0){
        localPortSpinBox->setMinimum(0);
        localPortSpinBox->setValue(localPortSuppliedByUser);
        localPortSpinBox->setDisabled(true);
    } else {
        localPortSpinBox->setMinimum(1024);
        localPortSpinBox->setValue(7755);
    }

    QLabel *label=new QLabel(QString("Local Address:"));
    label->setFixedWidth(120);

    widget->layout()->addWidget(label);
    widget->layout()->addWidget(localAddressComboBox);
    widget->layout()->addWidget(localPortSpinBox);
    this->layout()->addWidget(widget);

    widget=new QWidget();
    widget->setLayout(new QHBoxLayout());
    widget->layout()->setContentsMargins(0, 0, 0, 0);

    remoteAddressLineEdit = new QLineEdit(remoteAddressSuppliedByUser);
    remoteAddressLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    if (!remoteAddressSuppliedByUser.isEmpty()){
        remoteAddressLineEdit->setReadOnly(true);
    }

    remotePortSpinBox = new QSpinBox();
    remotePortSpinBox->setFixedWidth(100);
    remotePortSpinBox->setMaximum(65535);
    if (remotePortSuppliedByUser > 0){
        remotePortSpinBox->setMinimum(0);
        remotePortSpinBox->setValue(remotePortSuppliedByUser);
        remotePortSpinBox->setDisabled(true);
    } else {
        remotePortSpinBox->setMinimum(1024);
        remotePortSpinBox->setValue(7755);
    }

    label=new QLabel(QString("Remote Address:"));
    label->setFixedWidth(120);

    widget->layout()->addWidget(label);
    widget->layout()->addWidget(remoteAddressLineEdit);
    widget->layout()->addWidget(remotePortSpinBox);
    this->layout()->addWidget(widget);

    widget=new QWidget();
    widget->setLayout(new QHBoxLayout());
    widget->layout()->setContentsMargins(0, 0, 0, 0);

    this->layout()->addWidget(widget);

    socket=NULL;
}

QTFTPWidget::~QTFTPWidget()
{
    if (socket) delete socket;
}

bool QTFTPWidget::bindSocket()
{
    // SEE IF WE ALREADY HAVE A BOUND SOCKET
    // AND IF SO, WE NEED TO DELETE IT
    if (socket != NULL) {
        delete socket;
    }

    // CREATE A NEW SOCKET
    socket=new QUdpSocket();

    // AND SEE IF WE CAN BIND IT TO A LOCAL IP ADDRESS AND PORT
    if (localAddressSuppliedByUser.isEmpty()){
        return(socket->bind(QHostAddress(localAddressComboBox->currentText()), localPortSpinBox->value()));
    } else {
        return(socket->bind(QHostAddress(localAddressSuppliedByUser), localPortSuppliedByUser));
    }
}

QString QTFTPWidget::remoteAddressString()
{
    return(remoteAddressLineEdit->text());
}

QString QTFTPWidget::localAddressString()
{
    return(localAddressComboBox->currentText());
}

int QTFTPWidget::remotePortNumber()
{
    return(remotePortSpinBox->value());
}

int QTFTPWidget::localPortNumber()
{
    return(localPortSpinBox->value());
}

bool QTFTPWidget::putByteArray(QString filename, QByteArray transmittingFile)
{
    // BIND OUR LOCAL SOCKET TO AN IP ADDRESS AND PORT
    if (!bindSocket()) {
        lastErrorString = socket->errorString();
        return(false);
    }

    // MAKE A LOCAL COPY OF THE REMOTE HOST ADDRESS AND PORT NUMBER
    QHostAddress hostAddress(QHostAddress(remoteAddressLineEdit->text()));
    int portNumber = remotePortSpinBox->value();

    // CREATE REQUEST PACKET AND SEND TO HOST
    // WAIT UNTIL MESSAGE HAS BEEN SENT, QUIT IF TIMEOUT IS REACHED
    QByteArray reqPacket=putFilePacket(filename);
    if (socket->writeDatagram(reqPacket, hostAddress, portNumber) != reqPacket.length()){
        lastErrorString = QString("did not send packet to host :(  %1").arg(socket->errorString());
        return(false);
    }

    // CREATE PACKET COUNTERS TO KEEP TRACK OF MESSAGES
    unsigned short incomingPacketNumber=0;
    unsigned short outgoingPacketNumber=0;

    // NOW WAIT HERE FOR INCOMING DATA
    bool messageCompleteFlag=false;
    while (1){
        // WAIT FOR AN INCOMING PACKET
        if (socket->hasPendingDatagrams() || socket->waitForReadyRead(10000)){
            // ITERATE HERE AS LONG AS THERE IS ATLEAST A
            // PACKET HEADER'S WORTH OF DATA TO READ
            QByteArray incomingDatagram;
            incomingDatagram.resize(socket->pendingDatagramSize());
            socket->readDatagram(incomingDatagram.data(), incomingDatagram.length());

            // MAKE SURE FIRST BYTE IS 0
            char *buffer=incomingDatagram.data();
            char zeroByte=buffer[0];
            if (zeroByte != 0x00) {
                lastErrorString = QString("Incoming packet has invalid first byte (%1).").arg((int)zeroByte);
                return(false);
            }

            // READ UNSIGNED SHORT PACKET NUMBER USING LITTLE ENDIAN FORMAT
            // FOR THE INCOMING UNSIGNED SHORT VALUE BUT BIG ENDIAN FOR THE
            // INCOMING DATA PACKET
            unsigned short incomingMessageCounter;
            *((char*)&incomingMessageCounter+1)=buffer[2];
            *((char*)&incomingMessageCounter+0)=buffer[3];

            // CHECK INCOMING MESSAGE ID NUMBER AND MAKE SURE IT MATCHES
            // WHAT WE ARE EXPECTING, OTHERWISE WE'VE LOST OR GAINED A PACKET
            if (incomingMessageCounter == incomingPacketNumber){
                incomingPacketNumber++;
            } else {
                lastErrorString = QString("error on incoming packet number %1 vs expected %2").arg(incomingMessageCounter).arg(incomingPacketNumber);
                return(false);
            }

            // CHECK THE OPCODE FOR ANY ERROR CONDITIONS
            char opCode=buffer[1];
            if (opCode != 0x04) { /* ack packet should have code 4 and should be ack+1 the packet we just sent */
                lastErrorString = QString("Incoming packet returned invalid operation code (%1).").arg((int)opCode);
                return(false);
            } else {
                // SEE IF WE NEED TO SEND ANYMORE DATA PACKETS BY CHECKING END OF MESSAGE FLAG
                if (messageCompleteFlag) break;

                // SEND NEXT DATA PACKET TO HOST
                QByteArray transmitByteArray;
                transmitByteArray.append((char)0x00);
                transmitByteArray.append((char)0x03); // send data opcode
                transmitByteArray.append(*((char*)&outgoingPacketNumber+1));
                transmitByteArray.append(*((char*)&outgoingPacketNumber));

                // APPEND DATA THAT WE WANT TO SEND
                int numBytesAlreadySent=outgoingPacketNumber*MAXPACKETSIZE;
                int bytesLeftToSend=transmittingFile.length()-numBytesAlreadySent;
                if (bytesLeftToSend < MAXPACKETSIZE){
                    messageCompleteFlag=true;
                    if (bytesLeftToSend > 0){
                        transmitByteArray.append((transmittingFile.data()+numBytesAlreadySent), bytesLeftToSend);
                    }
                } else {
                    transmitByteArray.append((transmittingFile.data()+numBytesAlreadySent), MAXPACKETSIZE);
                }

                // SEND THE PACKET AND MAKE SURE IT GETS SENT
                if (socket->writeDatagram(transmitByteArray, hostAddress, portNumber) != transmitByteArray.length()){
                    lastErrorString = QString("did not send data packet to host :(  %1").arg(socket->errorString());
                    return(false);
                }

                // NOW THAT WE'VE SENT AN ACK SIGNAL, INCREMENT SENT MESSAGE COUNTER
                outgoingPacketNumber++;
            }
        } else {
            lastErrorString = QString("No message received from host :(  %1").arg(socket->errorString());
            return(false);
        }
    }
    lastErrorString = QString("no error");
    return(true);

}

bool QTFTPWidget::getByteArray(QString filename, QByteArray *requestedFile)
{
    // BIND OUR LOCAL SOCKET TO AN IP ADDRESS AND PORT
    if (!bindSocket()) {
        lastErrorString = socket->errorString();
        return(false);
    }

    // MAKE A LOCAL COPY OF THE REMOTE HOST ADDRESS AND PORT NUMBER
    QHostAddress hostAddress(QHostAddress(remoteAddressLineEdit->text()));
    int portNumber = remotePortSpinBox->value();

    // CLEAN OUT ANY INCOMING PACKETS
    while (socket->hasPendingDatagrams()){
        QByteArray byteArray;
        byteArray.resize(socket->pendingDatagramSize());
        socket->readDatagram(byteArray.data(), byteArray.length());
    }

    // CREATE REQUEST PACKET AND SEND TO HOST
    // WAIT UNTIL MESSAGE HAS BEEN SENT, QUIT IF TIMEOUT IS REACHED
    QByteArray reqPacket=getFilePacket(filename);
    if (socket->writeDatagram(reqPacket, hostAddress, portNumber) != reqPacket.length()){
        lastErrorString =  QString("did not send packet to host :(  %1").arg(socket->errorString());
        return(false);
    }

    // CREATE PACKET COUNTERS TO KEEP TRACK OF MESSAGES
    unsigned short incomingPacketNumber=1;
    unsigned short outgoingPacketNumber=1;

    // NOW WAIT HERE FOR INCOMING DATA
    bool messageCompleteFlag=false;
    while (!messageCompleteFlag){
        // WAIT FOR AN INCOMING PACKET
        if (socket->hasPendingDatagrams() || socket->waitForReadyRead(10000)){
            // ITERATE HERE AS LONG AS THERE IS ATLEAST A
            // PACKET HEADER'S WORTH OF DATA TO READ
            QByteArray incomingDatagram;
            incomingDatagram.resize(socket->pendingDatagramSize());
            socket->readDatagram(incomingDatagram.data(), incomingDatagram.length());

            // MAKE SURE FIRST BYTE IS 0
            char *buffer=incomingDatagram.data();
            char zeroByte=buffer[0];
            if (zeroByte != 0x00) {
                lastErrorString = QString("Incoming packet has invalid first byte (%1).").arg((int)zeroByte);
                return(false);
            }

            // READ UNSIGNED SHORT PACKET NUMBER USING LITTLE ENDIAN FORMAT
            // FOR THE INCOMING UNSIGNED SHORT VALUE BUT BIG ENDIAN FOR THE
            // INCOMING DATA PACKET
            unsigned short incomingMessageCounter;
            *((char*)&incomingMessageCounter+1)=buffer[2];
            *((char*)&incomingMessageCounter+0)=buffer[3];

            // CHECK INCOMING MESSAGE ID NUMBER AND MAKE SURE IT MATCHES
            // WHAT WE ARE EXPECTING, OTHERWISE WE'VE LOST OR GAINED A PACKET
            if (incomingMessageCounter == incomingPacketNumber){
                incomingPacketNumber++;
            } else {
                lastErrorString = QString("error on incoming packet number %1 vs expected %2").arg(incomingMessageCounter).arg(incomingPacketNumber);
                return(false);
            }

            // COPY THE INCOMING FILE DATA
            QByteArray incomingByteArray(&buffer[4], incomingDatagram.length()-4);

            // SEE IF WE RECEIVED A COMPLETE 512 BYTES AND IF SO,
            // THEN THERE IS MORE INFORMATION ON THE WAY
            // OTHERWISE, WE'VE REACHED THE END OF THE RECEIVING FILE
            if (incomingByteArray.length() < MAXPACKETSIZE){
                messageCompleteFlag=true;
            }

            // APPEND THE INCOMING DATA TO OUR COMPLETE FILE
            requestedFile->append(incomingByteArray);

            // CHECK THE OPCODE FOR ANY ERROR CONDITIONS
            char opCode=buffer[1];
            if (opCode != 0x03) { /* ack packet should have code 3 (data) and should be ack+1 the packet we just sent */
                lastErrorString = QString("Incoming packet returned invalid operation code (%1).").arg((int)opCode);
                return(false);
            } else {
                // SEND PACKET ACKNOWLEDGEMENT BACK TO HOST REFLECTING THE INCOMING PACKET NUMBER
                QByteArray ackByteArray;
                ackByteArray.append((char)0x00);
                ackByteArray.append((char)0x04);
                ackByteArray.append(*((char*)&incomingMessageCounter+1));
                ackByteArray.append(*((char*)&incomingMessageCounter));

                // SEND THE PACKET AND MAKE SURE IT GETS SENT
                if (socket->writeDatagram(ackByteArray, hostAddress, portNumber) != ackByteArray.length()){
                    lastErrorString = QString("did not send ack packet to host :(  %1").arg(socket->errorString());
                    return(false);
                }

                // NOW THAT WE'VE SENT AN ACK SIGNAL, INCREMENT SENT MESSAGE COUNTER
                outgoingPacketNumber++;
            }
        } else {
            lastErrorString = QString("No message received from host :(  %1").arg(socket->errorString());
            return(false);
        }
    }
    return(true);
}

QByteArray QTFTPWidget::getFilePacket(QString filename)
{
    QByteArray byteArray(filename.toLatin1());
    byteArray.prepend((char)0x01); // OPCODE
    byteArray.prepend((char)0x00);
    byteArray.append((char)0x00);
    byteArray.append(QString("octet").toLatin1()); // MODE
    byteArray.append((char)0x00);

    return(byteArray);
}

QByteArray QTFTPWidget::putFilePacket(QString filename)
{
    QByteArray byteArray((char)0x00);
    byteArray.append((char)0x02); // OPCODE
    byteArray.append(filename.toLatin1());
    byteArray.append((char)0x00);
    byteArray.append(QString("octet").toLatin1()); // MODE
    byteArray.append((char)0x00);

    return(byteArray);
}
