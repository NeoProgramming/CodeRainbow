DEFINES += CR_LIBRARY
DEFINES += QT_CODE

DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += d:/Libs/boost_1_69_0

MOC_DIR	= build/.moc
RCC_DIR	= build/.rcc
OBJECTS_DIR = build/.obj
UI_DIR	= ui

LIBS += -Ld:/Libs/boost_1_69_0/stage/lib

SOURCES += \
    ../src/qtplugin/crplugin.cpp \
    ../src/qtplugin/crplugincore.cpp \
    ../src/qtplugin/outputpane.cpp \
    ../src/qtplugin/infopane.cpp \
    ../src/qtplugin/contentview.cpp \
    ../src/qtplugin/contentwidgetfactory.cpp \
    ../src/qtplugin/crmodel.cpp \
    ../src/qtplugin/tabbar.cpp \
    ../src/qtplugin/toolbar.cpp \
    ../src/qtplugin/croptionspage.cpp \
    ../src/qtplugin/croptionswidget.cpp \
    ../src/qtplugin/crsettings.cpp \
    ../src/core/crtree.cpp \
    ../src/core/crnode.cpp \
    ../src/core/projectloader.cpp \
    ../src/core/kw.cpp \
    ../src/core/markmaker.cpp \
    ../src/core/mcmt.cpp \
    ../src/core/cruser.cpp \
    ../src/tools/ct.cpp \
    ../src/tools/tdefs.cpp \
    ../src/tools/utf.cpp \
    ../src/pugixml/pugixml.cpp \
    ../src/ted/text_encoding_detect.cpp \
    ../src/gui/gui.cpp \
    ../src/gui/tbstyle.cpp \
    ../src/gui/text.cpp \
    ../src/gui/dialogs/editrecentdlg.cpp \
    ../src/gui/dialogs/linkdlg.cpp \
    ../src/gui/dialogs/markupdlg.cpp \
    ../src/gui/dialogs/nodedlg.cpp \
    ../src/gui/dialogs/selnodedlg.cpp \
    ../src/gui/dialogs/newprojectdlg.cpp \
    ../src/gui/dialogs/prjpropsdlg.cpp \
    ../src/gui/widgets/colormenu.cpp \
    ../src/gui/editor/noteedit.cpp \
    ../src/gui/editor/memoform.cpp \
    ../src/gui/widgets/colorbutton.cpp

HEADERS += \
    ../src/qtplugin/crplugin.h \
    ../src/qtplugin/cr_global.h \
    ../src/qtplugin/crconstants.h \
    ../src/qtplugin/crplugincore.h \
    ../src/qtplugin/outputpane.h \
    ../src/qtplugin/infopane.h \
    ../src/qtplugin/contentview.h \
    ../src/qtplugin/contentwidgetfactory.h \
    ../src/qtplugin/crmodel.h \
    ../src/qtplugin/tabbar.h \
    ../src/qtplugin/toolbar.h \
    ../src/qtplugin/croptionspage.h \
    ../src/qtplugin/croptionswidget.h \
    ../src/qtplugin/crsettings.h \
    ../src/core/core.h \
    ../src/core/crtree.h \
    ../src/core/crnode.h \
    ../src/core/projectloader.h \
    ../src/core/loc.h \
    ../src/core/token.h \
    ../src/core/nlexer.h \
    ../src/core/kw.h \
    ../src/core/mcmt.h \
    ../src/core/mark.h \
    ../src/core/markmaker.h \
    ../src/core/parse.h \
    ../src/core/cruser.h \
    ../src/core/scan.h \
    ../src/tools/base_defs.h \
    ../src/tools/ct.h \
    ../src/tools/utf.h \
    ../src/tools/tfbase.h \
    ../src/pugixml/pugixml.hpp \
    ../src/ted/text_encoding_detect.h \
    ../src/gui/gui.h \
    ../src/gui/tbstyle.h \
    ../src/gui/text.h \
    ../src/gui/dialogs/editrecentdlg.h \
    ../src/gui/dialogs/linkdlg.h \
    ../src/gui/dialogs/markupdlg.h \
    ../src/gui/dialogs/newprojectdlg.h \
    ../src/gui/dialogs/nodedlg.h \
    ../src/gui/dialogs/selnodedlg.h \
    ../src/gui/dialogs/prjpropsdlg.h \
    ../src/gui/widgets/colormenu.h \
    ../src/gui/editor/noteedit.h \
    ../src/gui/editor/memoform.h \
    ../src/gui/widgets/colorbutton.h

FORMS += \
    ../src/qtplugin/croptionswidget.ui \
    ../src/gui/dialogs/editrecentdlg.ui \
    ../src/gui/dialogs/linkdlg.ui \
    ../src/gui/dialogs/markupdlg.ui \
    ../src/gui/dialogs/newprojectdlg.ui \
    ../src/gui/dialogs/nodedlg.ui \
    ../src/gui/dialogs/selnodedlg.ui \
    ../src/gui/dialogs/prjpropsdlg.ui \
    ../src/gui/editor/memoform.ui
	
RESOURCES += ../src/cr.qrc	

# Qt Creator linking

#debug {
#isEmpty(IDE_SOURCE_TREE): IDE_SOURCE_TREE = "d:/PRG/qtcsrc451vs15d"
#isEmpty(IDE_BUILD_TREE): IDE_BUILD_TREE = "d:/PRG/qtcsrc451vs15d/build"
#} else:release {
isEmpty(IDE_SOURCE_TREE): IDE_SOURCE_TREE = "../../../qtcsrc451vs15r"
isEmpty(IDE_BUILD_TREE): IDE_BUILD_TREE = "../../../qtcsrc451vs15r/build"
#}

## uncomment to build plugin into user config directory
## <localappdata>/plugins/<ideversion>
##    where <localappdata> is e.g.
##    "%LOCALAPPDATA%\QtProject\qtcreator" on Windows Vista and later
##    "$XDG_DATA_HOME/data/QtProject/qtcreator" or "~/.local/share/data/QtProject/qtcreator" on Linux
##    "~/Library/Application Support/QtProject/Qt Creator" on OS X
USE_USER_DESTDIR = yes

###### If the plugin can be depended upon by other plugins, this code needs to be outsourced to
###### <dirname>_dependencies.pri, where <dirname> is the name of the directory containing the
###### plugin's sources.

QTC_PLUGIN_NAME = CodeRainbow
QTC_LIB_DEPENDS += \
#    cplusplus \
    extensionsystem

QTC_PLUGIN_DEPENDS += \
    coreplugin \
    texteditor \
    projectexplorer \
#    cppeditor \
#    cpptools

QTC_PLUGIN_RECOMMENDS += \
    # optional plugin dependencies. nothing here at this time

###### End _dependencies.pri contents ######

include($$IDE_SOURCE_TREE/src/qtcreatorplugin.pri)
