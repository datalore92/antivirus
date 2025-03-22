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

LIBS += -lwinhttp

CONFIG += static

# Put all build artifacts in separate directories
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/ui

# Removed cleanup command to avoid post-link errors
# win32:QMAKE_POST_LINK += \
#     del /Q /F "$$OUT_PWD\\*.cpp" "$$OUT_PWD\\*.h" "$$OUT_PWD\\*.o" "$$OUT_PWD\\*.obj" \
#     "$$OUT_PWD\\*.c" "$$OUT_PWD\\Makefile*" "$$OUT_PWD\\.qmake.stash" 2>nul

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Deploy target: copies only the .exe and .dll files to a deploy folder.
deploy.files = $$OUT_PWD/$${TARGET}.exe $$OUT_PWD/*.dll
deploy.path = $$OUT_PWD/deploy
INSTALLS += deploy

win32:CONFIG(release):QMAKE_POST_LINK -= "cmd /C \"C:/Qt/6.8.2/mingw_64/bin/windeployqt.exe \"$$OUT_PWD/$${TARGET}.exe\" || exit 0\"\n"
win32:CONFIG(release):QMAKE_POST_LINK += $$quote(cmd /C "C:\\Qt\\6.8.2\\mingw_64\\bin\\windeployqt.exe --dir $$OUT_PWD\\release $$OUT_PWD\\release\\$${TARGET}.exe || exit 0")\n
