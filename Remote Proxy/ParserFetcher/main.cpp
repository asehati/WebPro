#include <QApplication>
#include "ServerParser.h"

/**
    A Simple driver for the ServerParser
    
    @author Ali Sehati
    @version 1.0
*/
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ServerParser server;
    server.startServer();

    return app.exec();
}
