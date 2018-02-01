QT   += webkitwidgets network
HEADERS += \
    Server_parser.h \
    mynetworkaccessmanager.h

SOURCES += \
    main.cpp \
    Server_parser.cpp \
    mynetworkaccessmanager.cpp



unix: LIBS += -L$$PWD/../../../../usr/lib/ -ltar

INCLUDEPATH += $$PWD/../../../../usr/include
DEPENDPATH += $$PWD/../../../../usr/include

unix: PRE_TARGETDEPS += $$PWD/../../../../usr/lib/libtar.a
