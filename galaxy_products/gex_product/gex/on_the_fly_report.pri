macx {
}

SOURCES += \
    sources/on_the_fly_report/UI/ofr_manager.cpp \
    sources/on_the_fly_report/UI/section_report_settings.cpp \
    sources/on_the_fly_report/UI/button_collapse_settings.cpp \
    sources/on_the_fly_report/UI/item_report_settings.cpp \
    sources/on_the_fly_report/UI/global_report_settings.cpp \
    sources/on_the_fly_report/Treeview/tree_item_report.cpp \
    sources/on_the_fly_report/Treeview/tree_model_report.cpp \
    sources/on_the_fly_report/Model/chart_element.cpp \
    sources/on_the_fly_report/Model/report_element.cpp \
    sources/on_the_fly_report/Model/section_element.cpp \
    sources/on_the_fly_report/Model/wafermap_element.cpp \
    sources/on_the_fly_report/Model/composite.cpp \
    sources/on_the_fly_report/Model/component.cpp \
    sources/on_the_fly_report/Renderer/dummy_report_renderer.cpp \
    sources/on_the_fly_report/ofr_controller.cpp \
    sources/on_the_fly_report/Renderer/html_renderer.cpp \
    $$PWD/sources/on_the_fly_report/Renderer/html_table_renderer.cpp \
    $$PWD/sources/on_the_fly_report/Model/statistical_table.cpp

HEADERS += \
    sources/on_the_fly_report/UI/section_report_settings.h \
    sources/on_the_fly_report/UI/ofr_manager.h \
    sources/on_the_fly_report/UI/button_collapse_settings.h \
    sources/on_the_fly_report/UI/item_report_settings.h \
    sources/on_the_fly_report/UI/global_report_settings.h \
    sources/on_the_fly_report/Treeview/tree_model_report.h \
    sources/on_the_fly_report/Treeview/tree_item_report.h \
    sources/on_the_fly_report/Model/chart_element.h \
    sources/on_the_fly_report/Model/report_element.h \
    sources/on_the_fly_report/Model/section_element.h \
    sources/on_the_fly_report/Model/wafermap_element.h \
    sources/on_the_fly_report/Model/component.h \
    sources/on_the_fly_report/Model/composite.h \
    sources/on_the_fly_report/Renderer/basic_report_builder.h \
    sources/on_the_fly_report/Renderer/basic_report_builder.tcc \
    sources/on_the_fly_report/Renderer/dummy_report_renderer.h \
    sources/on_the_fly_report/ofr_controller.h \
    sources/on_the_fly_report/utility.h \
    sources/on_the_fly_report/Renderer/html_renderer.h \
    $$PWD/sources/on_the_fly_report/Renderer/html_table_renderer.h \
    $$PWD/sources/on_the_fly_report/Renderer/renderer_keys.h \
    $$PWD/sources/on_the_fly_report/Model/statistical_table.h

INCLUDEPATH += \
    sources/on_the_fly_report/UI \
    sources/on_the_fly_report/Treeview \
    sources/on_the_fly_report/Model \
    sources/on_the_fly_report/Renderer \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/pdf_director/sources

FORMS += \
    sources/on_the_fly_report/UI/ofr_manager.ui \
    sources/on_the_fly_report/UI/button_collapse_settings.ui \
    sources/on_the_fly_report/UI/section_settings.ui \
    sources/on_the_fly_report/UI/item_report_settings.ui \
    sources/on_the_fly_report/UI/global_report_settings.ui

# /!\ - Test code to remove ASAP
QMAKE_CXXFLAGS_WARN_ON = -Wno-unused-parameter
