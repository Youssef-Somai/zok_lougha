QT += core gui sql widgets charts network printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    activite.cpp \
    adherent.cpp \
    aiverifier.cpp \
    connexion.cpp \
    imagehandler.cpp \
    local.cpp \
    main.cpp \
    mainwindow.cpp \
    materiel.cpp \
    qrcodegen.cpp \
    qrcodegen_real.cpp \
    qrcodehelper.cpp \
    qrdecoder.cpp \
    qrscannerdialog.cpp \
    smtp.cpp

HEADERS += \
    activite.h \
    adherent.h \
    aiverifier.h \
    connexion.h \
    imagehandler.h \
    local.h \
    mainwindow.h \
    materiel.h \
    qrcodegen.h \
    qrcodegen_real.h \
    qrcodehelper.h \
    qrdecoder.h \
    qrscannerdialog.h \
    smtp.h

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
