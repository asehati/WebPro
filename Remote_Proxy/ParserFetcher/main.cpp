#include <QApplication>
#include "Server_parser.h"



int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Server_parser server;
    server.startServer();

    return app.exec();
}
