#include "parserFactory.h"
#include "parserFactoryPrivate.h"
#include "importSkyNPCsv.h"
#include "import_semi_g85.h"
#include "import_csv_skyworks.h"
#include "importMicron.h"
#include "importMicronSUM.h"
#include "importTriQuintRF.h"
#include "importTriQuintDC.h"
#include "importSkyworksIFF.h"
#include "importInphiPrestro.h"
#include "importInphiBBA.h"
#include "importWat.h"
#include "importWatAsmc.h"
#include "importWatSmic.h"
#include "importWatUmc.h"
#include "importWatTsmXml.h"
#include "importPcmDongbu.h"
#include "importWoburnCsv.h"
#include "importWoburnSECSIIMap.h"
#include "importSiTimeEtest.h"
#include "importMexicaliMap.h"
#include "importAquantiaPCM.h"
#include "import_fet_test.h"
#include "importVanguardPCM.h"
#include "import_pcm_hjtc.h"
#include "importSpinstand.h"
#include "importSipex.h"
#include "importSpektraLotSummary.h"
#include "import_fet_test_summary.h"
#include "import_spektra_datalog.h"
#include "importHvmSummary.h"
#include "importVishayASE.h"
#include "importVishayASESummary.h"
#include "importVishayATM.h"
#include "importAtdfToStdf.h"
#include "import_stdf.h"
#include "import_kla.h"
#include "import_vishay_ase_qa.h"
#include "importWatGlobalFoundry.h"

namespace GS
{
namespace Parser
{

ParserFactory * ParserFactory::mInstance = NULL;

ParserAbstract *ParserFactory::CreateParser(ParserType lType)
{
    return mPrivate->CreateParser(lType);
}

ParserAbstract *ParserFactory::CreateParser(const std::string& lFileName)
{
    return mPrivate->CreateParser(QString::fromStdString(lFileName));
}

ParserType ParserFactory::FindParserType(const std::string &lFileName)
{
    return mPrivate->FindParserType(QString::fromStdString(lFileName));
}

ParserFactory *ParserFactory::GetInstance()
{
    if (mInstance == NULL)
    {
        mInstance = new ParserFactory();

        // Register parsers
        mInstance->Initialize();
    }

    return mInstance;
}

void ParserFactory::ReleaseInstance()
{
    if (mInstance)
    {
        delete mInstance;
        mInstance = NULL;
    }
}

void ParserFactory::Initialize()
{
}

ParserFactory::ParserFactory()
    : mPrivate(new ParserFactoryPrivate)
{
    // Register all available parser here
    // i.e.
    mPrivate->RegisterParser(/*TriQuintDCtoSTDF, */new ParserFactoryItem<TriQuintDCToSTDF>());
    mPrivate->RegisterParser(/*TriQuintRFtoSTDF, */new ParserFactoryItem<TriQuintRFToSTDF>());
    mPrivate->RegisterParser(/*typeSkyNPCSV, */new ParserFactoryItem<CGSkyNPCsvtoSTDF>());
    mPrivate->RegisterParser(/*typeInphiPresto, */new ParserFactoryItem<InphiPrestotoSTDF>());
    mPrivate->RegisterParser(/*typeInphiBBA, */new ParserFactoryItem<InphiBBASTDF>());
    mPrivate->RegisterParser(/*typeSemi_G85, */new ParserFactoryItem<CGSEMI_G85toSTDF>());
    mPrivate->RegisterParser(/*typeSkyFasterCSV, */new ParserFactoryItem<CGCSVSkyworkstoSTDF>());
    mPrivate->RegisterParser(/*typeMicrontoSTDF, */new ParserFactoryItem<MicronToSTDF>());
    mPrivate->RegisterParser(/*typeMicronSumtoSTDF, */new ParserFactoryItem<MicronSumToSTDF>());
    mPrivate->RegisterParser(/*typeSkyworksIFF, */new ParserFactoryItem<SkyworksIFFToSTDF>());
    mPrivate->RegisterParser(new ParserFactoryItem<WatToStdf>());
    mPrivate->RegisterParser(new ParserFactoryItem<WatAsmcToStdf>());
    mPrivate->RegisterParser(new ParserFactoryItem<WatSmicToStdf>());
    mPrivate->RegisterParser(new ParserFactoryItem<WatUmcToStdf>());
    mPrivate->RegisterParser(new ParserFactoryItem<WatTsmXmlToStdf>());
    mPrivate->RegisterParser(new ParserFactoryItem<PCM_DONGBUtoSTDF>());
    mPrivate->RegisterParser(new ParserFactoryItem<WoburnCSVtoSTDF>());
    mPrivate->RegisterParser(new ParserFactoryItem<WoburnSECSIIMap>());
    mPrivate->RegisterParser(new ParserFactoryItem<SiTimeEtesttoSTDF>());
    mPrivate->RegisterParser(new ParserFactoryItem<AquantiaPCMtoSTDF>());
    mPrivate->RegisterParser(new ParserFactoryItem<MexicaliMap>());
    mPrivate->RegisterParser(new ParserFactoryItem<VanguardPCMToStdf>());
    mPrivate->RegisterParser(new ParserFactoryItem<CGPcmHjtctoSTDF>());
    mPrivate->RegisterParser(new ParserFactoryItem<CGFET_TESTtoSTDF>());
    mPrivate->RegisterParser(new ParserFactoryItem<CGSpinstandtoSTDF>());
    mPrivate->RegisterParser(new ParserFactoryItem<SipextoSTDF>());
    mPrivate->RegisterParser(new ParserFactoryItem<SpektraLotSummary>());
    mPrivate->RegisterParser(new ParserFactoryItem<FetTestSummaryToStdf>());
    mPrivate->RegisterParser(new ParserFactoryItem<CGSpektraDatalogtoSTDF>());
    mPrivate->RegisterParser(new ParserFactoryItem<ImportHvmSummary>());
    mPrivate->RegisterParser(new ParserFactoryItem<VishayASEtoSTDF>());
    mPrivate->RegisterParser(new ParserFactoryItem<VishayASESummarytoSTDF>());
    mPrivate->RegisterParser(new ParserFactoryItem<VishayATMtoSTDF>());
    mPrivate->RegisterParser(new ParserFactoryItem<AtdfToStdf>());
    mPrivate->RegisterParser(new ParserFactoryItem<STDFtoSTDF>());
    mPrivate->RegisterParser(new ParserFactoryItem<ImportKLAMap>());
    mPrivate->RegisterParser(new ParserFactoryItem<VishayASEQAtoSTDF>());
    mPrivate->RegisterParser(new ParserFactoryItem<WatGlobalFoundryEtesttoSTDF>());
}

ParserFactory::~ParserFactory()
{
    if (mPrivate)
    {
        delete mPrivate;
        mPrivate = 0;
    }
}

}
}
