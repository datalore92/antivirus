QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    scanworker.cpp \
    engine/signature.c \
    engine/scanner.c

HEADERS += \
    mainwindow.h \
    scanworker.h \
    engine/signature.h \
    engine/scanner.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
