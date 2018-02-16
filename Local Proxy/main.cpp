#include <QApplication>
#include <QStringList>
#include "client.h"

/**
    A Simple driver for the Client
    This program should be called with the target URL as its first argument.
    The second argument is the name of the statistic file that holds the timing
    measurements.
    
    @author Ali Sehati
    @version 1.0
*/
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QStringList arguments = app.arguments();
    arguments.takeFirst();

    Client client;
    client.setUrl(arguments.at(0));
    client.setStatFileName(arguments.at(1));
    client.connectToServer();

    return app.exec();
}
