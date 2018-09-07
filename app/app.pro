TARGET = visclient
QT = quickcontrols2 websockets

SOURCES = main.cpp visclient.cpp
HEADERS = visclient.h

RESOURCES += \
    visclient.qrc

include(app.pri)
