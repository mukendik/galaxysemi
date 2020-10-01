DEFINES += MODULE=GQTL_PARSER
QT       += xml
QT       -= gui

TEMPLATE = lib
CONFIG += debug_and_release
CONFIG+= staticlib

DEFINES += GQTL_PARSER_LIBRARY
QT += widgets

include($(DEVDIR)/galaxy_common.pri)

DESTDIR = $$PWD/../lib/$$OSFOLDER
TARGET = $$gexTargetName(gqtl_parser)

SOURCES += \
    sources/importSipex.cpp \
    sources/importMicronSUM.cpp \
    common/parserBinning.cpp \
    sources/importSkyNPCsv.cpp \
    common/parserBase.cpp \
    common/parserFactory.cpp \
    common/parserFactoryPrivate.cpp \
    common/parameterDictionary.cpp \
    sources/import_semi_g85.cpp \
    common/parserParameter.cpp \
    sources/import_csv_skyworks.cpp \
    sources/micronParserBase.cpp\
    sources/importMicron.cpp \
    sources/importTriQuintRF.cpp \
    sources/importTriQuintDC.cpp \
    sources/importSkyWorksIFF.cpp \
    sources/importInphiPresto.cpp \
    sources/importWat.cpp \
    sources/importWatAsmc.cpp \
    sources/importWatSmic.cpp \
    sources/importWatUmc.cpp \
    sources/importWatTsmXml.cpp \
    sources/importPcmDongbu.cpp \
    sources/importWoburnCsv.cpp \
    sources/importWoburnSECSIIMap.cpp \
    sources/importSiTimeEtest.cpp \
    sources/importMexicaliMap.cpp \
    sources/importAquantiaPCM.cpp \
    sources/importVanguardPCM.cpp \
    sources/importInphiBBA.cpp \
    sources/import_pcm_hjtc.cpp \
    common/parserWafer.cpp \
    sources/importSpinstand.cpp \
    sources/importSpektraLotSummary.cpp \
    sources/import_fet_test_summary.cpp \
    sources/import_spektra_datalog.cpp \
    sources/importHvmSummary.cpp \
    sources/importVishayASE.cpp \
    sources/importVishayASESummary.cpp \
    sources/importVishayATM.cpp \
    common/parserPromisFile.cpp \
    sources/import_fet_test.cpp \
    common/dl4_db.cpp \
    common/dl4_tools.cpp \
    sources/importAtdfToStdf.cpp \
    sources/import_stdf.cpp \
    common/vishay_stdf_record_overwrite.cpp \
    sources/import_kla.cpp \
    common/converter_external_file.cpp \
    sources/import_vishay_ase_qa.cpp \
    sources/importWatGlobalFoundry.cpp

HEADERS +=\
    include/importSipex.h \
    include/importMicronSUM.h \
    common/parserBinning.h \
    common/progressHandlerAbstract.h \
    include/importSkyNPCsv.h \
    include/importConstants.h \
    common/parserBase.h \
    common/parserFactory.h \
    common/parserFactoryPrivate.h \
    common/parserAbstract.h \
    common/parameterDictionary.h \
    common/gqtlParserGlobal.h \
    include/import_semi_g85.h \
    common/parserParameter.h \
    include/import_csv_skyworks.h \
    include/importMicron.h \
    include/micronParserBase.h \
    include/importTriQuintRF.h \
    include/importTriQuintDC.h \
    include/importSkyworksIFF.h \
    include/importInphiPrestro.h \
    include/importWat.h \
    include/importWatAsmc.h \
    include/importWatSmic.h \
    include/importWatUmc.h \
    include/importWatTsmXml.h \
    include/importPcmDongbu.h \
    include/importWoburnCsv.h \
    include/importWoburnSECSIIMap.h \
    include/importSiTimeEtest.h \
    include/importMexicaliMap.h \
    include/importAquantiaPCM.h \
    include/importVanguardPCM.h \
    include/importInphiBBA.h \
    include/import_pcm_hjtc.h \
    common/parserWafer.h \
    include/importSpinstand.h \
    include/importSpektraLotSummary.h \
    common/converter_external_file.h \
    include/import_fet_test_summary.h \
    include/import_spektra_datalog.h \
    include/importHvmSummary.h \
    include/importVishayASE.h \
    include/importVishayATM.h \
    common/parserPromisFile.h \
    include/import_fet_test.h \
    common/dl4_db.h \
    common/dl4_tools.h \
    include/importAtdfToStdf.h \
    include/import_stdf.h \
    common/vishay_stdf_record_overwrite.h \
    include/import_kla.h \
    include/import_vishay_ase_qa.h \
    include/importWatGlobalFoundry.h

INCLUDEPATH = $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_parser/include \
              $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
              $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
              $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_parser/common

INCLUDEPATH += $(QTSRCDIR)/qtbase/src/3rdparty/zlib

CONFIG(debug, debug|release) {
        OBJECTS_DIR = $$PWD/debug/$$OSFOLDER
}
else {
        OBJECTS_DIR = $$PWD/release/$$OSFOLDER
}

INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_parser/include \
               $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_parser/common \
               $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
               $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include

INCLUDEPATH += \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_std_utils/sources \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_std_utils/sources/string_utils \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/common \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/common/promis_interpreter \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/lvm_ws \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/lvm_ft \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/hvm_ws \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/hvm_ft \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/promis_interpreter/hvm_ft \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/promis_interpreter/hvm_wt \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/promis_interpreter/lvm_wt \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/promis_interpreter/lvm_ft \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/promis_interpreter/lvm_ft_subcon_data \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/promis_interpreter/lvm_ft_lotlist
