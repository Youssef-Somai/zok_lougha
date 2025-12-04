QT += core gui sql widgets charts network printsupport multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

INCLUDEPATH += header
DEPENDPATH += header

SOURCES += \
    source/aiverifier.cpp \
    source/connection.cpp \
    source/imagehandler.cpp \
    source/main.cpp \
    source/mainwindow.cpp \
    source/materiel.cpp \
    source/qrcodegen.cpp \
    source/qrcodegen_real.cpp \
    source/qrcodehelper.cpp \
    source/qrdecoder.cpp \
    source/qrscannerdialog.cpp

HEADERS += \
    header/aiverifier.h \
    header/connection.h \
    header/imagehandler.h \
    header/mainwindow.h \
    header/materiel.h \
    header/qrcodegen.h \
    header/qrcodegen_real.hpp \
    header/qrcodehelper.h \
    header/qrdecoder.h \
    header/qrscannerdialog.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    ressource.qrc

DISTFILES += \
    ../../Pictures/image/logor.png \
    style.qss

# Déploiement
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
