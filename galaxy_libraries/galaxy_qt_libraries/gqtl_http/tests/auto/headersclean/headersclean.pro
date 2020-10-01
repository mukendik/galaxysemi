QT = core testlib http

include($(DEVDIR)/galaxy_warnings.pri)

include($${QT.core.sources}/../../tests/auto/other/headersclean/headersclean.pri)
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
