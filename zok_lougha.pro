QT += core gui sql widgets charts network printsupport
QT += multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Include paths for organized structure
INCLUDEPATH += header
INCLUDEPATH += quirc

# Source files from main branch
SOURCES += \
    activite.cpp \
    adherent.cpp \
    connexion.cpp \
    local.cpp \
    smtp.cpp

# Source files from Materiel branch
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
    source/qrcodegen.cpp \
    source/qrcodegen_real.cpp \
    source/qrcodehelper.cpp \
    source/qrscannerdialog.cpp \
    source/qrdecoder.cpp

# Headers from main branch
HEADERS += \
    activite.h \
    adherent.h \
    connexion.h \
    local.h \
    smtp.h

# Headers from Materiel branch
HEADERS += \
    header/connection.h \
    header/mainwindow.h \
    header/materiel.h \
    header/imagehandler.h \
    header/aiverifier.h \
    header/qrcodegen.h \
    header/qrcodegen_real.hpp \
    header/qrcodehelper.h \
    header/qrscannerdialog.h \
    header/qrdecoder.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    ressource.qrc

DISTFILES += \
    image/logor.png \
    style.qss

# Deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

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
