/**
    A Simple driver for the ServerGraph.
    By defining "MIX_APPROACH" this server will serve a random set of
    requests with web-engine and the rest of the requests with 
    resource lists. In the case of "MIX_APPROACH", this program 
    should be called with one argument. This argument represents 
    the number of requests that should be served with the web-engine.
    
    @author Ali Sehati
    @version 1.1
*/

// uncomment the following line to use a mix of 
// resource list and web-engine approaches
//#define MIX_APPROACH
#include "ServerGraph.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Server_graph server;

#ifdef MIX_APPROACH
    QStringList arguments = app.arguments();
    arguments.takeFirst();

	int ratio = arguments.at(0).toInt();
    server.setRandListMissRatio(ratio);
    server.randListBuilder(ratio);
#endif

    server.startServer();
    return app.exec();
}
