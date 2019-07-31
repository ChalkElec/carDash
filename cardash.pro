TEMPLATE = app
TARGET = CarDash

QT += qml quick serialport
QT -= gui
CONFIG += c++11

SOURCES += src/main.cpp \
           src/hardware.cpp \
           src/xmlparser.cpp

HEADERS += src/hardware.h \
           src/xmlparser.h

RESOURCES += cardash.qrc

