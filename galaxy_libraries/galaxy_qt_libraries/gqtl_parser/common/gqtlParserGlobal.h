#ifndef GQTL_PARSER_GLOBAL_H
#define GQTL_PARSER_GLOBAL_H

//#if defined(_WIN32_)
//#if defined(GQTL_PARSER_LIBRARY)
//#  define GQTL_PARSERSHARED_EXPORT Q_DECL_EXPORT
//#else
//#  define GQTL_PARSERSHARED_EXPORT Q_DECL_IMPORT
//#endif
//#else
//#  define GQTL_PARSERSHARED_EXPORT
//#endif

//#include "importWoburnSECSIIMap.h"
//#include "importMexicaliMap.h"


namespace GS
{
namespace Parser
{
    ///
    /// \brief The ConverterStatus enum
    ///
    enum ConverterStatus
    {
        ConvertSuccess = 0,
        ConvertWarning,
        ConvertDelay,
        ConvertError
    };

    ///
    /// \brief The ParserType enum
    ///
    enum ParserType
    {
        typeUnknown,
        typeSkyNPCSV,
        typeSemi_G85,
        typeSkyFasterCSV,
        typeMicron,
        typeMicronSum,
        typeTriQuintRF,
        typeTriQuintDC,
        typeSkyworksIFF,
        typeInphiPresto,
        typeInphiBBA,
        type7C7,
        type93ktab,
        typeAcco,
        typeAdvantestT2000,
        typeAmida,
        typeAsl1000,
        typeCsm,
        typeCsm2,
        typeCsmc,
        typeCsmcSpdm,
        typeCsv,
        typeCsvSkyworks,
        typeDl4,
        typeEagleDatalog,
        typeWat,
        typeWatAsmc,
        typeWatSMic,
        typeWatUmc,
        typeWatTsmXml,
        typePCMDongbu,
        typeWoburnCsv,
        typeWoburnSECSIIMap,
        typeSiTimeEtest,
        typeAquantiaPCM,
        typeMexicaliMap,
        typeVanguardPcm,
        typePcmHjtc,
        typeSpinstand,
        typeSipex,
        typeSpektraLotSummary,
        typeFetTestSummary,
        typeFetTest,
        typeSpektraDatalog,
        typeHvmSummary,
        typeVishayASE,
        typeVishayASESummary,
        typeVishayATM,
        typeAtdfToStdf,
        typeStdf,
        typeKLA,
        typeVishayASEQA,
        typeWatGlobalFoundry
    };


    template <ParserType>
    struct ParserDef
    {
    };

}
}

#endif // GQTL_PARSER_GLOBAL_H
