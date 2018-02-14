/**
    A Simple driver for the ServerParser
    
    @author Ali Sehati
    @version 1.1
*/

#include <QApplication>
#include "Server_parser.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ServerParser server;
    server.startServer();

    return app.exec();
}
