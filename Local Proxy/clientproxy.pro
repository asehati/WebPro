QT += webkitwidgets network

HEADERS += \
    client.h \
    mynetworkaccessmanager.h

SOURCES += \
    main.cpp \
    client.cpp \
    mynetworkaccessmanager.cpp


#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../usr/lib/release/ -ltar
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../usr/lib/debug/ -ltar
#else:unix: LIBS += -L$$PWD/../../../../usr/lib/ -ltar

#INCLUDEPATH += $$PWD/../../../../usr/include
#DEPENDPATH += $$PWD/../../../../usr/include

#win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../../../usr/lib/release/libtar.a
#else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../../../usr/lib/debug/libtar.a
#else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../../../usr/lib/release/tar.lib
#else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../../../usr/lib/debug/tar.lib
#else:unix: PRE_TARGETDEPS += $$PWD/../../../../usr/lib/libtar.a

unix:!macx: LIBS += -L$$PWD/../../../../usr/lib/ -ltar

INCLUDEPATH += $$PWD/../../../../usr/include
DEPENDPATH += $$PWD/../../../../usr/include

unix:!macx: PRE_TARGETDEPS += $$PWD/../../../../usr/lib/libtar.a
