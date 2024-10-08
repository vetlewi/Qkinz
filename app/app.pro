include(../defaults.pri)

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport webenginewidgets

TARGET = Qkinz
TEMPLATE = app

DESTDIR = $$BUILDDIR/app
OBJECTS_DIR = $$BUILDDIR/app
MOC_DIR = $$BUILDDIR/app
RCC_DIR = $$BUILDDIR/app
UI_DIR = $$BUILDDIR/app
CONFIG += sdk_no_version_check
macx {
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 14
    QMAKE_CXXFLAGS += -std=c++20 -O3
    LIBS += -dead_strip
    ICON = ../resources/media/Qkinz.icns
}


SOURCES += main.cpp

LIBS += -L$$BUILDDIR/src -lkinzgui

linux {
    binfile.files += $$BUILDDIR/app/$$TARGET
    binfile.path += /usr/bin/
    icon.files += $$PWD/../Qkinz.png
    icon.path += /usr/share/$$TARGET
    desktop.path = /usr/share/applications/
    desktop.files = $$PWD/../Qkinz.desktop
    INSTALLS += binfile
    INSTALLS += icon
    INSTALLS += desktop
}

QMAKE_CLEAN += 	$$BUILDDIR/app/*.o \
				$$BUILDDIR/app/moc_* \
				$$BUILDDIR/app/qrc_* \
				$$BUILDDIR/app/ui_*
