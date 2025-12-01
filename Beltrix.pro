TEMPLATE = app

# Qt6 modules
QT += qml quick widgets network

SOURCES += main.cpp

RESOURCES += qml.qrc

CONFIG += console c++17

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    auth.h

# Target name
TARGET = onedrive-linux-client

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

OTHER_FILES += \
    android/AndroidManifest.xml
