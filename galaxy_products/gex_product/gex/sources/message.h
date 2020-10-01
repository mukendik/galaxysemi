/******************************************************************************!
 * \file message.h
 ******************************************************************************/
#ifndef GEX_MESSAGE
#define GEX_MESSAGE

#include <QObject>
#include <QString>

namespace GS
{
namespace Gex
{

#define MESSAGE_REQUEST 8
/******************************************************************************!
 * \class Message
 ******************************************************************************/
class Message : public QObject
{
public:
    /*!
     * \brief request the user if he accept or no the proposition
     */
    static void request(const QString title, const QString message, bool& accepted);
    /*!
     * \fn information
     */
    static void information(const QString title, const QString message);
    /*!
     * \fn warning
     */
    static void warning(const QString title, const QString message);
    /*!
     * \fn critical
     */
    static void critical(const QString title, const QString message);

private:
    Q_DISABLE_COPY(Message)

    /*!
     * \fn Message
     * \brief Constructor
     */


    Message();
    /*!
     * \fn send
     */
    static void send(const int level,
                     const QString& title,
                     const QString& message,
                     bool& accepted);
};

}
}

#endif
