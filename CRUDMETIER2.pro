QT += core gui sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += charts
QT += core gui sql widgets charts network
QT += core gui sql widgets charts printsupport
CONFIG += c++11





CONFIG += c++17

SOURCES += \
    activite.cpp \
    adherent.cpp \
    connexion.cpp \
    local.cpp \
    main.cpp \
    mainwindow.cpp \
    smtp.cpp

HEADERS += \
    activite.h \
    adherent.h \
    connexion.h \
    local.h \
    mainwindow.h \
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
