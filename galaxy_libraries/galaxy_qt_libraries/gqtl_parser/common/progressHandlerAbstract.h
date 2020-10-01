#ifndef PROGRESSHANDLERABSTRACT_H
#define PROGRESSHANDLERABSTRACT_H

#include <string>
#include "gqtlParserGlobal.h"

namespace GS
{
namespace Parser
{
class /*GQTL_PARSERSHARED_EXPORT*/ ProgressHandlerAbstract
{
public:


    virtual ~ProgressHandlerAbstract(){}
    /**
     * \fn void Start(const long minValue, const long maxValue)
     * \brief Start the tracking of the progress
     * \param minValue: minimum value
     *        maxValue: maximum value
     */
    virtual void Start(const long minValue, const long maxValue) = 0;

    /**
     * \fn void SetValue(const long value)
     * \brief Set the value of the progress
     * \param value: value of the progess
     */
    virtual void SetValue(const long value) = 0;

    /**
     * \fn void Increment()
     * \brief Increment the progress
     */
    virtual void Increment() = 0;

    /**
     * \fn void Finish()
     * \brief Finish the progress
     */
    virtual void Finish() = 0;

    /**
     * \fn void SetMessage(const std::string message)
     * \brief Set the message
     * \param message: The message to display
     */
    virtual void SetMessage(const std::string message) = 0;

protected:
    long mMaxValue;   /// \param the max value of the progress
    long mMinValue;   /// \param the min value of the progress
};

}
}

#endif // PROGRESSHANDLERABSTRACT_H
