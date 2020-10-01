#ifndef PAT_ENGINE_H
#define PAT_ENGINE_H

#include <QScriptValue>
#include <QObject>
#include "pat_info.h"

namespace GS
{
namespace Gex
{
#ifdef GCORE15334

class PATProcessing;
#endif
class PATEnginePrivate;

class PATEngine : public QObject
{
    friend class EnginePrivate;

    Q_OBJECT
    Q_DISABLE_COPY(PATEngine)

    // Private destructor to prevent several instantiation
    PATEngine(QObject* parent);
    virtual ~PATEngine();

    static PATEngine * mInstance;

public:

    /// \brief Return the instance of the pat engine
    static PATEngine& GetInstance();

    /// \brief Return a pointer to the context
    CPatInfo* GetContext() const;

    /// \brief Delete the context if exist
    void DeleteContext();

    /// \brief Create a context. Delete the exist one if exists.
    /// \return true, if correctly created.
    bool CreateContext();

    /// \brief This method is part of the Script API
    ///        This function convert recipe from cvs format to json format
    /// \param CsvRecipeName the input file
    ///        JsonRecipeName the output file. if it is empty, we use <CsvRecipeName without the .csv extension>.json
    /// \return error message
    Q_INVOKABLE QString ConvertPatRecipe(const QString &CsvRecipeName,
                             const QString &JsonRecipeName = "");

    /*!
      @brief    Returns the error message
      */
    Q_INVOKABLE const QString&      GetErrorMessage() const;

#ifdef GCORE15334

    /*!
      @brief    This method is part of the Script API.
                Build a PAT report based on the current PAT context and the given PATProcessing keys.

      @param    lFields     Pointer on the PAT processing keys

      @return   "Ok" string if the report build succeed, otherwise a string beginning with "Error"
      */
    Q_INVOKABLE QString BuildPATReport(GS::Gex::PATProcessing * lFields);

    /*!
      @brief    Build a PAT report based on the current PAT context and the given PATProcessing keys.

      @param    lPatmanFile         STDF patman file used for the report
      @param    lFields             PAT processing keys
      @param    lSites              List of sites to be included in the report
      @param    lErrorMessage       Potential error message given by the method.
      @param    lOpenReportViewer   Specifies whether the report has to be shown or not.

      @return   Return true if the report generation succeed, otherwise returns false.
      */
    bool BuildPATReport(const QString& lPatmanFile, GS::Gex::PATProcessing& lFields,
                           QList<int> lSites, bool lOpenReportViewer = false);
#endif


    /*!
      @brief    Create a Z PAT composite file (also called ZMap) from a list of input files and using
                a given recipe.

      @param    lRecipeFile         Recipe file to use
      @param    lFiles              List of input files
      @param    lCompositeFile      File name of the output composite file

      @return   Return true if the composite file is generated, otherwise returns false.
      */
    bool CreateZPATCompositeFile(const QString& lRecipeFile, const QStringList& lFiles, QString& lCompositeFile);

    /*! GCORE-1100
      @brief    Create a ZPAT composite file through a ScriptValue
      @return   Return the path ot the output file or "error...."
      */
    Q_INVOKABLE QString CreateZPATCompositeFile(const QString& lRecipeFile, const QScriptValue& lFiles);

    /*!
      @brief    Export a Z PAT composite file (also called ZMap) to a SINF file.

      @param    lCompositeFile      Composite file to export
      @param    lSINF               File name of the ouptut SINF file
      @param    lDeviceName         Device name for the composite
      @param    lLotID              Lot ID for the composite
      @param    lWaferID            Wafer ID for the of the composite

      @return   Return true if the export succeed, otherwise returns false.
      */
    Q_INVOKABLE bool ExportZPATCompositeFileToSINF(const QString& lCompositeFile, const QString& lSINF,
                                          const QString& lDeviceName, const QString& lLotID, const QString& lWaferID);

private:

    PATEnginePrivate * mPrivate;
};

}
}

#endif // PAT_ENGINE_H
