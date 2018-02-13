/**
	A Simple driver for the Profiler
	This program should be called with the target URL as its argument
	
    @author Ali Sehati
    @version 1.1
*/

#include "profiler.h"
#include <QStringList>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QStringList arguments = app.arguments();
    arguments.takeFirst();

    Profiler profiler;
    profiler.setUrl(arguments.at(0));
    profiler.loadPage();

    return app.exec();
}
