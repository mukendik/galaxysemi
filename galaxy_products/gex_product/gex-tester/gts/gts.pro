include($(DEVDIR)/galaxy_warnings.pri)

TEMPLATE	= app
LANGUAGE	= C++

CONFIG += qt release


SOURCES	+= main.cpp
FORMS	= gts_mainwindow_base.ui
IMAGES	= images/filenew \
	images/fileopen \
	images/filesave \
	images/print \
	images/undo \
	images/redo \
	images/editcut \
	images/editcopy \
	images/editpaste \
	images/searchfind \
	images/exit.png \
	images/new_station.png \
	images/green_sphere.png \
	images/red_sphere.png \
	images/galaxy_logo_22.png \
	images/galaxy_logo_32.png



































































































































































unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}


