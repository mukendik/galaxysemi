#ifndef PARAMETERDICTIONARY_H
#define PARAMETERDICTIONARY_H

#include <QString>
#include <QMap>
#include <QStringList>

namespace GS
{
namespace Parser
{

class ParameterDictionary
{
public:

    /**
     * \fn ParameterDictionary()
     * \brief Constructor
     */
    ParameterDictionary();

    /**
     * \fn ParameterDictionary(const QString &paramRepositoryName)
     * \brief Constructor
     */
    ParameterDictionary(const QString& paramRepositoryName);

    /**
     * \fn ParameterDictionary(const QString &fileName, const QString &paramRepositoryName)
     * \brief Constructor
     */
    ParameterDictionary(const QString &fileName, const QString &paramRepositoryName);

    /// These function load the txt file to increment the test number
    /**
     * \fn void LoadParameterIndexTable(void)
     * \brief Load all parameters in the table
     */
    void LoadParameterIndexTable(void);

    /**
     * \fn void DumpParameterIndexTable(void)
     * \brief Put all the parameters in the parameter's file
     */
    void DumpParameterIndexTable(void);

    /**
     * \fn void UpdateParameterIndexTable(QString strParamName)
     * \brief Update the file of parameters: if the param name exists, do nothing
     *          else, add it to the file
     * \param strParamName The parameter to to add.
     */
    long UpdateParameterIndexTable(const QString &paramName);

    /**
     * \fn QStringList GetFullParametersList() const
     * \brief Getteur of the parameter list
     * \return Return the parameter list
     */
    QStringList GetFullParametersList() const;

    /**
     * \fn bool GetNewParameterAdded() const
     * \brief return if a new parameter has been found
     * \return if a new parameter has been found, else return false
     */
    bool            GetNewParameterFound() const;

// Just change the name of the previous function after transfering all of them in the library
//    bool        LoadDictionary();
//    bool        DumpDictionary();

    int             GetTestNumber(const QString& parameterName) const;
//    bool        ContainsParameter(const QString& parameterName) const;
//    int         InsertParameter(const QString& parameterName);

    void            SetFileName         (const QString &fileName)        { mFileName = fileName; }
    void            SetRepositoryName   (const QString &repositoryName)  { mParamRepositoryName = repositoryName; }



private:


    QString             mFileName;              /// \param
    QString             mParamRepositoryName;   /// \param  The name of the directory which contains the the param files
//    QMap<QString, int>  mDictionary;            /// \param
//    QStringList         mParameters;            /// \param
//    bool                mUpdated;               /// \param

    bool                mNewParameterFound;     /// \param set to true if CSV file has a Parameter name not in our reference table=> makes it to be updated
    QStringList         mFullParametersList;    /// \param Complete list of ALL CSV parameters known.
};

}
}

#endif // PARAMETERDICTIONARY_H
