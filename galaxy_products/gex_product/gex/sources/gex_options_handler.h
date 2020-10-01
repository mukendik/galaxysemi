#ifndef GEX_OPTIONS_HANDLER_H
#define GEX_OPTIONS_HANDLER_H

#include "report_options_type_definition.hpp"
#include "gex_options_map.h"
#include <set>


class CReportOptions;
namespace GS
{
namespace Gex
{

/*! \class OptionsHandler
    \brief The OptionsHandler class allows to fill the content of an options map.
*/
class OptionsHandler
{
public:

    /*!
      @brief    Constructs a default options handler with an empty options map
      */
    OptionsHandler();

    /*!
      @brief    Copy constructor
      */
    OptionsHandler(const OptionsHandler& other);

    /*!
      @brief    Destructor
      */
    ~OptionsHandler();

    /*!
      @brief    Assignement operator
      */
    OptionsHandler&                 operator=(const OptionsHandler& other);

    /*!
      @brief    Returns the current options map.

      @sa       SetOptionsMap, ClearOptionsMap
      */
    const OptionsMap&               GetOptionsMap() const;

    /*!
      @brief    Returns the current options type definition.
      */
    const OptionsTypeDefinition&    GetOptionsTypeDefinition() const;


    /*!
      @brief    Set options map of the engine to the given \a optionsMap

      @param    optionsMap   New options map to use

      @sa       GetOptionsMap, ClearOptionsMap
      */
    bool                            SetOptionsMap(const OptionsMap &optionsMap);

    /*!
      @brief    Returns true if the options type definition file has been
                successfully loaded

      @sa       GetOptionsTypeDefinition
      */
    bool                            IsReady() const;

    /*!
      @brief    Set the option to the given \a value. If the option does not exist,
                it will be added to the map

      @param    section The section name of the option
      @param    option  The name of the option
      @param    value   The value of the option

      @sa       SetSpecificFlagOption
      */
    bool                            SetOption(const QString& section,
                                              const QString& option,
                                              const QString& value);

    /*!
      @brief    Set the specific option of a flag option \a lFlag to true/false
                depending on \a lEnabled

      @param    lSection    The section name of the option
      @param    lOption     The name of the option
      @param    lFlag       The flag name of the option
      @param    lEnabled    The status of the flag, true if it is enabled,
                            otherwise false.

      @sa       SetOption
      */
    bool                            SetSpecificFlagOption(const QString& lSection,
                                                          const QString& lOption,
                                                          const QString& lFlag,
                                                          bool lEnabled);

    /*!
      @brief    Clear the contents of the options map and makes it empty

      @sa       GetOptionsMap, SetOptionsMap
      */
    void                            ClearOptionsMap();

    /*!
      @brief    Reset the options map to the default depending on the given \a
                lDefault

      @note     Standard options are always reset to their default value.
                Flag options are reset to their default value when \a lDefault
                is set to true, otherwise they are cleared.

      @param    lDefault    Reset mode for flag option, if true flag options are
                            reset, otherwise they are cleared.

      @sa       GetOptionsMap, SetOptionsMap
      */
    void                            Reset(bool lDefault);

    /*!
      @brief    Write all registered options to the given \a hFile.

      @param    hFile    Handle to the csl file to write.
      */
    bool                            WriteOptionsToFile(FILE* hFile) const;


    static bool IsOptionReportRebuildLess(const std::string &section, const std::string &name);
    static void ActiveNeedToRebuild(const std::string &section, const std::string &name);

    static bool sNeedToRebuild;


     void UpdateReportOption(CReportOptions &lReportOption);
    /*!
      @brief    Map storing the options
      */
    OptionsMap              mOptionsMaps;

    /*!
      @brief    Handle the options type definition in order to check the validity
                of the options.
      */
    OptionsTypeDefinition	mOptionsTypeDefinition;

    /*!
     * \brief handle the options that doesn't spark the report to be rebuild
     */
    typedef std::set< std::pair<std::string, std::string> > ContainerOptionsWithNoRebuil;
    static ContainerOptionsWithNoRebuil CreateOptionsWithNoRebuildReport();
    static ContainerOptionsWithNoRebuil mOptionsWithNoRebuildReport;
};

}   // namespace Gex
}   // namespace GS

#endif // GEX_OPTIONS_HANDLER_H
