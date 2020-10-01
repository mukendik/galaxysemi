#ifndef VFEI_CONFIG_H
#define VFEI_CONFIG_H

#include <QString>

namespace GS
{
namespace Gex
{

/*! \class  VFEIConfig
    \brief  The VFEIConfig class provides the configuration of the VFEI server
            (connection and processing settings).
*/
class VFEIConfig
{
public:

    /*!
      @brief    Constructs a default VFEI Configuration object
      */
    VFEIConfig();                           // Constructor

    /*!
      @brief    Constructs a copy of the given \a config.
    */
    VFEIConfig(const VFEIConfig& config);	// Constructor

    /*!
      @brief    Destroys the VFEI configuration object
    */
    ~VFEIConfig();

    /*!
      @brief    Assigns another configuration \a config to this object, and
                returns a reference to this object.
    */
    VFEIConfig& operator=(const VFEIConfig& config);

    bool            LoadFromFile(const QString& lFileName, QString& lErrorMessage);
    bool            IsValid(QString& lErrorMessage) const;

    /*!
      @brief    Returns the socket port to use
    */
    int             GetSocketPort() const;

    /*!
      @brief    Returns the maximum number of instances of VFEI client connected.
    */
    int             GetMaxAllowedInstances() const;

    /*!
      @brief    Returns true if the input file has to be deleted after processing,
                otherwise returns false.
    */
    bool            GetDeleteInput() const;

    /*!
      @brief    Returns the path of the Production Recipe folder
    */
    const QString&  GetProdRecipeFolder() const;

    /*!
      @brief    Returns the path of the Engineering Recipe folder
    */
    const QString&  GetEngRecipeFolder() const;

    /*!
      @brief    Returns the path of the Input STDF folder
    */
    const QString&  GetInputSTDFFolder() const;


    /*!
      @brief    Returns the path of the Input Map folder
    */
    const QString&  GetInputMapFolder() const;


    /*!
      @brief    Returns the path of the folder where output STDF files are
                generated
    */
    const QString&  GetOutputSTDFFolder() const;

    /*!
      @brief    Returns the path of the folder where output MAP files are
                generated
    */
    const QString&  GetOutputMapFolder() const;

    /*!
      @brief    Returns the path of the folder where Report file is generated
    */
    const QString&  GetOutputReportFolder() const;

    /*!
      @brief    Returns the path of the folder where files created moved on
                error situation
    */
    const QString&  GetErrorFolder() const;

protected:

    /*!
      @brief    Clear the VFEI configuration object
    */
    void            Clear();

private:

    int             mSocketPort;			// Socket port#
    int             mMaxAllowedInstances;   // Maximum concurrent instances allowed
    bool            mDeleteInput;			// Delete input files after processing.
    QString         mProdRecipeFolder;      // Production recipe location
    QString         mEngRecipeFolder;       // Engineering recipe location
    QString         mInputSTDFFolder;       // STDF input folder
    QString         mInputMapFolder;        // MAP input folder
    QString         mOutputSTDFFolder;      // Output STDF folder
    QString         mOutputMapFolder;       // Output MAP folder
    QString         mOutputReportFolder;    // Output REPORT folder
    QString         mErrorFolder;           // Output ERROR folder (where files created moved on error situation)
};

}   // namespace Gex
}   // namespace GS

#endif // VFEI_CONFIG_H
