#include "Server_graph.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);


    Server_graph server;

    // this argument represents the number of requests that should be missed from the set of resource lists
#ifdef MIX_APPROACH
    QStringList arguments = app.arguments();
    arguments.takeFirst();

    server.setRandListMissRatio(arguments.at(0).toInt());
    server.randListBuilder(arguments.at(0).toInt());

#endif

    server.startServer();

    return app.exec();
}
