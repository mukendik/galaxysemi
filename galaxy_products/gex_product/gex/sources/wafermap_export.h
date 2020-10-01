#ifndef WAFERMAP_EXPORT_H
#define WAFERMAP_EXPORT_H

#include "gex_report.h"
#include "patman_lib.h"
#include <QString>

class WafermapExport
{

public:
    WafermapExport(){}
    ~WafermapExport(){}

    // Create a Wafermap output file
    // Note: return error code
    int		CreateWafermapOutput(QString &strOutputWafermapPath, QString &strWaferFileName, const QString &strOutputWafermapFormat,
                                 int iBinType,int iNotch,int iPosX,int iPosY,bool bRotateWafer,
                                 QString &strErrorMessage,QString strCustomer="",QString strSupplier="");
    /*!
      * \brief Create a Wafermap output file: TSMC ASCII inkless file
      *	Format:
      *	TSMC
      *	TMN178
      *	A45234-03
      *	A4523403.TSM
      *	...............112221...............
      *	............111111111111............
      *	etc...
      *	Note: The wafer notch MUST always be at located at '6' oclock (DOWN)
      * \param	strOutputWafermapPath
      * \param	strWaferFileFullName
      *	\param	iNotch
      *	\param	iPosX
      *	\param	iPosY
      *	\param	bRotateWafer
      *	\param	strErrorMessage
      *	\param	bSaveAs
      *	\param	iGroupID
      *	\param	iFileID
      *	\return	error code
    */
    static bool initScriptEngineProperties(GexScriptEngine*	poScriptEngine);


    static int	CreateWafermapOutput_TSMC(QString &strOutputWafermapPath,QString &strWaferFileFullName,
                                      int iNotch,int iPosX,int iPosY,bool bRotateWafer,
                                      QString &strErrorMessage,
                                      bool bSaveAs=false,int iGroupID=0,int iFileID=0);

    //! \brief Create a Wafermap output file: G85 (Semi85 inkless assembly map format / Amkor)
    //! \return return an error code
    static int	CreateWafermapOutput_G85(QString &strOutputWafermapPath,
                                         QString &strWaferFileFullName,
                                     int iNotch, int iPosX, int iPosY,
                                     bool bRotateWafer, QString &strErrorMessage,
                                     QString strCustomer="", QString strSupplier="",
                                     bool bXmlFormat=false, bool bSaveAs=false,
                                     int iGroupID=0, int iFileID=0);
    static int	CreateWafermapOutput_STIF(QString &strOutputWafermapPath,QString &strWaferFileFullName,
                                      int iNotch,int iPosX,int iPosY,bool bRotateWafer,
                                      QString &strErrorMessage,bool bSaveAs=false,int iGroupID=0,int iFileID=0);
    static int	CreateWafermapOutput_SEMI142(bool isInteger2,QString &strOutputWafermapPath,QString &strWaferFileFullName,
                                         int iNotch,int iPosX,int iPosY,bool bRotateWafer,
                                         QString &strErrorMessage,QString strCustomer="",QString strSupplier="",
                                         bool bSaveAs=false,int iGroupID=0,int iFileID=0);

    static int	CreateWafermapOutput_SINF(QString &strOutputWafermapPath,QString &strWaferFileFullName,
                                      int iNotch,int iPosX,int iPosY,bool bRotateWafer,
                                      QString &strErrorMessage,
                                          const GexTbPatSinf &lSinfInfo,
                                      bool bSaveAs=false,int iGroupID=0,int iFileID=0);
    static int	CreateWafermapOutput_HTML(QString &strOutputWafermapPath,QString &strWaferFileFullName,
                                      int iNotch,int iPosX,int iPosY,bool bRotateWafer,
                                      QString &strErrorMessage,bool bSaveAs=false,int iGroupID=0,int iFileID=0);
    static int	CreateWafermapOutput_TELP8(QString &strOutputWafermapPath,QString &strWaferFileFullName,
                                       int iNotch,int iPosX,int iPosY,bool bRotateWafer,QString &strErrorMessage);
    static int	CreateWafermapOutput_IMAGE_PNG(QString &strOutputWafermapPath,
                                               QString &strWaferFileFullName,
                                               int iNotch,int iPosX,int iPosY,
                                               bool bRotateWafer,
                                               QString &strErrorMessage,
                                               CGexReport::wafermapMode eWafermapMode,
                                               bool bSaveAs=false, int iGroupID=0,
                                               int iFileID=0);

    static int	CreateWafermapOutput_LaurierDieSort1D(QString &strOutputWafermapPath,
                                                      QString &strWaferFileFullName,
                                                      int iNotch,int iPosX,
                                                      int iPosY,bool bRotateWafer,
                                                      QString &strErrorMessage,
                                                      bool bSaveAs=false,
                                                      int iGroupID=0,int iFileID=0);

    // Error codes returned by Stdf functions
    enum WaferExportErrorCode
    {
        NoError,            // No error
        ProcessDone,        // No error: file processed
        NoFile,             // Input file doesn't exist!
        WriteError,         // Failed writing output file
        WaferExport         // Failed to export wafer into maps
    };
private:
    static QString getCustomWaferMapFilename();
    static bool isValidDieAround(int iLine,int iCol,int iWaferSizeX,int iWaferSizeY,CWaferMap *ptEnlargedMap);

    GexTbPatSinf	m_cSinfInfo;			// Holds KLA/INF info that are used if requested to generate a SINF file

};
#endif // WAFERMAP_EXPORT_H
