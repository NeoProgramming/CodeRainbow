TEMPLATE  = app
TARGET    = CodeRainbow
CONFIG   += c++11 qt thread stl
QT       += core widgets gui 

DESTDIR = ../

DEPENDPATH += .

MOC_DIR	= build/.moc
RCC_DIR	= build/.rcc
OBJECTS_DIR = build/.obj
UI_DIR	= ui

debug {
  DESTDIR = Debug
  DEFINES += _DEBUG
} else:release {
  DESTDIR = Release
}

win32:RC_FILE = ./src/cr.rc

SOURCES += \
    ./src/main.cpp \
    ./src/core/crtree.cpp \
    ./src/core/crnode.cpp \
    ./src/core/projectloader.cpp \
    ./src/gui/gui.cpp \
    ./src/gui/childwindow.cpp \
    ./src/gui/mainwindow.cpp \
    ./src/gui/infopanel.cpp \
    ./src/gui/ctrlpanel.cpp \
    ./src/gui/tabwidget.cpp \
    ./src/gui/dialogs/newprojectdlg.cpp \
    ./src/gui/editor/highlighter.cpp \
    ./src/gui/editor/texteditwidget.cpp \
    ./src/pugixml/pugixml.cpp \
    ./src/gui/dialogs/markupdlg.cpp \
    ./src/gui/editor/noteedit.cpp \
    ./src/gui/dialogs/prjpropsdlg.cpp \
    ./src/core/mcmt.cpp \
    ./src/core/kw.cpp \
    ./src/tools/ct.cpp \
    ./src/tools/utf.cpp \
    ./src/gui/dialogs/linkdlg.cpp \
    src/gui/markmaker.cpp
    
HEADERS += \
    ./src/core/crtree.h \
    ./src/core/crnode.h \
    ./src/core/projectloader.h \
    ./src/gui/gui.h \
    ./src/gui/childwindow.h \
    ./src/gui/mainwindow.h \
    ./src/gui/infopanel.h \
    ./src/gui/ctrlpanel.h \
    ./src/gui/tabwidget.h \
    ./src/gui/dialogs/newprojectdlg.h \
    ./src/gui/editor/highlighter.h \
    ./src/gui/editor/texteditwidget.h \
    ./src/pugixml/pugixml.hpp \
    ./src/gui/dialogs/markupdlg.h \
    ./src/gui/editor/noteedit.h \
    ./src/gui/dialogs/prjpropsdlg.h \
    ./src/core/mcmt.h \
    ./src/core/nlexer.h \
    ./src/core/kw.h \
    ./src/tools/ct.h \
    ./src/tools/utf.h \
    ./src/tools/tfbase.h \
    ./src/base_defs.h \
    ./src/gui/dialogs/linkdlg.h \
    src/gui/markmaker.h
		
FORMS += \
    ./src/gui/mainwindow.ui \
    ./src/gui/infopanel.ui \
    ./src/gui/ctrlpanel.ui \
    ./src/gui/dialogs/newprojectdlg.ui \
    ./src/gui/dialogs/markupdlg.ui \
    ./src/gui/dialogs/prjpropsdlg.ui \
    ./src/gui/dialogs/linkdlg.ui



RESOURCES += ./src/cr.qrc
