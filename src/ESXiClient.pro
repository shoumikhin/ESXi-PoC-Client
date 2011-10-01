QT          -= core gui

TEMPLATE    = app

TARGET      = hav

DEFINES     += WITH_OPENSSL WITH_COOKIES

SOURCES     += main.cpp

INCLUDEPATH += ../lib

LIBS        = ../lib/libSoapVim.so -lgsoapssl++ -lssl -lcrypto
