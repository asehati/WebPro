#include "Server_parser.h"
#include "mynetworkaccessmanager.h"

Server_parser::Server_parser(QObject *parent) :
    QObject(parent)
{
    done = false;
    objCounter = 0;

    netAccess = new MyNetworkAccessManager();
    diskCache = new QNetworkDiskCache(this);
    diskCache->setCacheDirectory("batch");
    diskCache->clear();
    netAccess->setCache(diskCache);
    netAccess->clearAccessCache();

    webpage.setNetworkAccessManager(netAccess);

    webpage.settings()->clearIconDatabase();
    webpage.settings()->clearMemoryCaches();

    QObject::connect(netAccess, SIGNAL(finished(QNetworkReply *)),
                     this, SLOT(replyFinished(QNetworkReply *)));

    QObject::connect(&webpage, SIGNAL(loadFinished(bool)),
                     this, SLOT(pageLoadFinished(bool)));
}

void Server_parser::startServer()
{
    tcpServer = new QTcpServer(this);
    QObject::connect(tcpServer, SIGNAL(newConnection()), this, SLOT(on_newConnection()));

    if(!tcpServer->listen(QHostAddress::Any,PORT))
        qDebug() << "Could not listen to port: " << PORT;
}

void Server_parser::setUrl(QString myUrl)
{
    url.setUrl(myUrl);
}

void Server_parser::on_newConnection()
{
    done = false;
    objCounter = 0;    

    clientConnection = tcpServer->nextPendingConnection();
    QObject::connect(clientConnection, SIGNAL(disconnected()),
                     clientConnection, SLOT(deleteLater()));
    QObject::connect(clientConnection, SIGNAL(readyRead()), this, SLOT(readRequest()));

    QObject::connect(clientConnection, SIGNAL(disconnected()),
                     this, SLOT(on_disconnect()));
}

void Server_parser::readRequest()
{
    if (clientConnection->canReadLine())
    {
        QStringList tokens = QString(clientConnection->readLine()).split(QRegExp("[ \r\n][ \r\n]*"));
        if (tokens[0] == "GET")
        {
            qDebug() << "---- in ready read slot ----";
            qDebug() << tokens[1];
            setUrl(tokens[1]);
            loadPage();
        }
    }
}

void Server_parser::loadPage()
{
    webpage.settings()->clearMemoryCaches();
    webpage.settings()->clearIconDatabase();
    webpage.settings()->setAttribute(QWebSettings::JavascriptEnabled,true);
    diskCache->clear();
    webpage.mainFrame()->load(url);
}

void Server_parser::replyFinished(QNetworkReply * reply)
{
    objCounter++;
    QNetworkRequest req = reply->request();
    QUrl current_url = req.url();

    reply->deleteLater();

    if(current_url.isEmpty())
        return;

    if(done)
    {
        qDebug() << current_url.toString();
        return;
    }

    if (objCounter == 5)
    {

        buildTar();
        sendBatch();
        webpage.networkAccessManager()->clearAccessCache();
    }

}

void Server_parser::on_disconnect()
{
    qDebug() << "--------- in disconnect ---------";
    clientConnection->deleteLater();
}

void Server_parser::pageLoadFinished(bool k)
{
//    webpage.settings()->setAttribute(QWebSettings::JavascriptEnabled,false);

//    if(done)
//        return;

//     done = true;

    if (k)
    {       
        qDebug() << "-------------------------------" ;
    }

//    buildTar();
//    sendBatch();
}

void Server_parser::buildTar()
{
    const char tarFilename[] = "batch.tar";

    QFile file(tarFilename);

    if(file.exists())
        file.remove();

    TAR *pTar;

    tar_open(&pTar, tarFilename, NULL, O_WRONLY | O_CREAT, 0644, TAR_GNU);

    tar_append_tree(pTar, "batch/", ".");

    tar_close(pTar);
}

void Server_parser::sendBatch()
{
    QFile file("batch.tar");
    file.open(QIODevice::ReadOnly);
    QByteArray tarBlock = file.readAll();

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << (quint32)tarBlock.size();
    block.append(tarBlock);

    clientConnection->write(block);
    clientConnection->flush();

    file.close();

    clientConnection->disconnectFromHost();
}
