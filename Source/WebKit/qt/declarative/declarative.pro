TARGET  = qmlwebkitplugin
TARGETPATH = QtWebKit

TEMPLATE = lib
CONFIG += qt plugin

win32|mac:!wince*:!win32-msvc:!macx-xcode:CONFIG += debug_and_release

isEmpty(OUTPUT_DIR): OUTPUT_DIR = ../../..

QMLDIRFILE = $${_PRO_FILE_PWD_}/qmldir
copy2build.input = QMLDIRFILE
copy2build.output = $$OUTPUT_DIR/imports/$$TARGETPATH/qmldir
!contains(TEMPLATE_PREFIX, vc):copy2build.variable_out = PRE_TARGETDEPS
copy2build.commands = $$QMAKE_COPY ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
copy2build.name = COPY ${QMAKE_FILE_IN}
copy2build.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += copy2build

TARGET = $$qtLibraryTarget($$TARGET)
contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols

wince*:LIBS += $$QMAKE_LIBS_GUI

include(../../../WebKit.pri)

QT += declarative
qt5: QT += widgets

contains(QT_CONFIG, qtquick1): {
    QT += qtquick1
}

linux-* {
    # From Creator's src/rpath.pri:
    # Do the rpath by hand since it's not possible to use ORIGIN in QMAKE_RPATHDIR
    # this expands to $ORIGIN (after qmake and make), it does NOT read a qmake var.
    QMAKE_RPATHDIR = $$OUTPUT_DIR/lib $$QMAKE_RPATHDIR
    MY_RPATH = $$join(QMAKE_RPATHDIR, ":")

    QMAKE_LFLAGS += -Wl,-z,origin \'-Wl,-rpath,$${MY_RPATH}\'
    QMAKE_RPATHDIR =
} else {
    QMAKE_RPATHDIR = $$OUTPUT_DIR/lib $$QMAKE_RPATHDIR
}

SOURCES += qdeclarativewebview.cpp plugin.cpp
HEADERS += qdeclarativewebview_p.h

webkit2: DEFINES += HAVE_WEBKIT2

DESTDIR = $$OUTPUT_DIR/imports/$$TARGETPATH

target.path = $$[QT_INSTALL_IMPORTS]/$$TARGETPATH


qmldir.files += $$PWD/qmldir
qmldir.path +=  $$[QT_INSTALL_IMPORTS]/$$TARGETPATH

INSTALLS += target qmldir
