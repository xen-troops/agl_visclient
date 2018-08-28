TARGET = visclient
QT = quickcontrols2 websockets

SOURCES = main.cpp visclient.cpp
HEADERS = visclient.h

CONFIG += link_pkgconfig
PKGCONFIG += libhomescreen qlibwindowmanager

RESOURCES += \
    visclient.qrc

include(app.pri)
