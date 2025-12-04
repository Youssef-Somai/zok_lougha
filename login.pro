QT       += core gui
QT       += sql
QT       += multimedia multimediawidgets network
QT       += charts printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
INCLUDEPATH += header
INCLUDEPATH += quirc

SOURCES += \
    quirc/quirc.c \
    quirc/decode.c \
    quirc/identify.c \
    quirc/version_db.c \
    source/connection.cpp \
    source/main.cpp \
    source/mainwindow.cpp \
    source/materiel.cpp \
    source/imagehandler.cpp \
    source/aiverifier.cpp \
    source/qrcodegen_real.cpp \
    source/qrcodehelper.cpp \
    source/qrscannerdialog.cpp \
    source/qrdecoder.cpp

HEADERS += \
    header/connection.h \
    header/mainwindow.h \
    header/materiel.h \
    header/imagehandler.h \
    header/aiverifier.h \
    header/qrcodegen_real.hpp \
    header/qrcodehelper.h \
    header/qrscannerdialog.h \
    header/qrdecoder.h

FORMS += \
    mainwindow.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    image/logor.png \
    style.qss

RESOURCES += \
    ressource.qrc

# Copy config.ini to build directory if it exists
win32 {
    CONFIG_FILE = $$shell_quote($$shell_path($$PWD/config.ini))
    OUT_DIR = $$shell_quote($$shell_path($$OUT_PWD))
    exists($$PWD/config.ini) {
        QMAKE_POST_LINK += $$QMAKE_COPY $$CONFIG_FILE $$OUT_DIR $$escape_expand(\\n\\t)
    }
}
unix {
    exists($$PWD/config.ini) {
        QMAKE_POST_LINK += $$QMAKE_COPY $$shell_quote($$PWD/config.ini) $$shell_quote($$OUT_PWD) $$escape_expand(\\n\\t)
    }
}
