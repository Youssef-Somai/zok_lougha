#==============================================================================
# ZOK LOUGHA - Unified Project Configuration
#==============================================================================
# This project combines material management with QR code functionality
# and CRUD operations for members, activities, and locations.
#==============================================================================

QT += core gui sql
QT += multimedia multimediawidgets network
QT += charts printsupport widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = zok_lougha
TEMPLATE = app

#==============================================================================
# Include Paths
#==============================================================================
INCLUDEPATH += include
INCLUDEPATH += external/quirc

#==============================================================================
# Source Files
#==============================================================================
SOURCES += \
    # Main Application
    src/main.cpp \
    src/mainwindow.cpp \
    \
    # Material & QR Code Features
    src/materiel.cpp \
    src/connection.cpp \
    src/imagehandler.cpp \
    src/aiverifier.cpp \
    src/qrcodegen_real.cpp \
    src/qrcodehelper.cpp \
    src/qrscannerdialog.cpp \
    src/qrdecoder.cpp \
    \
    # CRUD Features (Members, Activities, Locations)
    src/adherent.cpp \
    src/activite.cpp \
    src/local.cpp \
    src/connexion.cpp \
    src/smtp.cpp \
    \
    # External Libraries
    external/quirc/quirc.c \
    external/quirc/decode.c \
    external/quirc/identify.c \
    external/quirc/version_db.c

#==============================================================================
# Header Files
#==============================================================================
HEADERS += \
    # Main Application
    include/mainwindow.h \
    \
    # Material & QR Code Features
    include/materiel.h \
    include/connection.h \
    include/imagehandler.h \
    include/aiverifier.h \
    include/qrcodegen_real.hpp \
    include/qrcodehelper.h \
    include/qrscannerdialog.h \
    include/qrdecoder.h \
    \
    # CRUD Features
    include/adherent.h \
    include/activite.h \
    include/local.h \
    include/connexion.h \
    include/smtp.h

#==============================================================================
# UI Forms
#==============================================================================
FORMS += \
    ui/mainwindow.ui

#==============================================================================
# Resources
#==============================================================================
RESOURCES += \
    resources/ressource.qrc

DISTFILES += \
    resources/images/logor.png \
    resources/images/logor.jpg \
    resources/style.qss \
    database/database_migration.sql \
    database/database_rollback.sql \
    config/config.ini.example \
    arduino/RFID/RFID.ino \
    arduino/general/arduino.ino \
    README.md

#==============================================================================
# Installation
#==============================================================================
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#==============================================================================
# Post-Build: Copy config.ini to build directory
#==============================================================================
win32 {
    CONFIG_FILE = $$shell_quote($$shell_path($$PWD/config/config.ini))
    OUT_DIR = $$shell_quote($$shell_path($$OUT_PWD))
    exists($$PWD/config/config.ini) {
        QMAKE_POST_LINK += $$QMAKE_COPY $$CONFIG_FILE $$OUT_DIR $$escape_expand(\\n\\t)
    }
}
unix {
    exists($$PWD/config/config.ini) {
        QMAKE_POST_LINK += $$QMAKE_COPY $$shell_quote($$PWD/config/config.ini) $$shell_quote($$OUT_PWD) $$escape_expand(\\n\\t)
    }
}
