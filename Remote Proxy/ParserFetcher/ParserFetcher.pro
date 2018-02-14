QT   += webkitwidgets network
HEADERS += \
    ServerParser.h \
    mynetworkaccessmanager.h

SOURCES += \
    main.cpp \
    ServerParser.cpp \
    mynetworkaccessmanager.cpp



unix: LIBS += -L$$PWD/../../../../usr/lib/ -ltar

INCLUDEPATH += $$PWD/../../../../usr/include
DEPENDPATH += $$PWD/../../../../usr/include

unix: PRE_TARGETDEPS += $$PWD/../../../../usr/lib/libtar.a
