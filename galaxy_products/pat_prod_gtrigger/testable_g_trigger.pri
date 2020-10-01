include($(DEVDIR)/galaxy_common.pri)

CONFIG(debug, debug|release) {
    G_TRIGGER_OBJECTS_DIR = $(DEVDIR)/galaxy_products/pat_prod_gtrigger/debug/$$OSFOLDER
}
else {
    G_TRIGGER_OBJECTS_DIR = $(DEVDIR)/galaxy_products/pat_prod_gtrigger/release/$$OSFOLDER
}