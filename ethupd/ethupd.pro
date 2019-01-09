TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_LFLAGS += -no-pie
SOURCES += main.cpp \
    eth.cpp \
    crc_32.cpp

HEADERS += \
    eth.h \
    crc_32.h
