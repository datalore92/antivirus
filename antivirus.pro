QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    scanworker.cpp \
    signature.c
    # scanner.c  <-- removed if not used

HEADERS += \
    mainwindow.h \
    scanworker.h \
    signature.h
    # scanner.h  <-- remove, if not used

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
