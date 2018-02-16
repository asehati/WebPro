#include "ServerParser.h"
#include "mynetworkaccessmanager.h"

/**
    Class ServerParser
	
    The server program which employs a web-engine to load webpages
    requested by the local proxy. After page load ends, the proxy packs all 
    the objects in one bundle and sends it to the local proxy.
    
    @author Ali Sehati
    @version 1.0
*/
ServerParser::ServerParser(QObject *parent) :
    QObject(parent)
{
    done = false;

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

/**
	This method starts the server. The server listens at port PORT 
	for requests coming from the client.
*/
void ServerParser::startServer()
{
    tcpServer = new QTcpServer(this);
    QObject::connect(tcpServer, SIGNAL(newConnection()), this, SLOT(on_newConnection()));

    if(!tcpServer->listen(QHostAddress::Any,PORT))
        qDebug() << "Could not listen to port: " << PORT;
}

/**
	This method sets the url that was embedded in client's request.
	
	@param myUrl The url associated with client requested page
*/
void ServerParser::setUrl(QString myUrl)
{
    url.setUrl(myUrl);
}

/**
	A Qt slot that is called every time a new connection establishment  
	request is received from client.
*/
void ServerParser::on_newConnection()
{
    done = false;   

    clientConnection = tcpServer->nextPendingConnection();
    QObject::connect(clientConnection, SIGNAL(disconnected()),
                     clientConnection, SLOT(deleteLater()));
    QObject::connect(clientConnection, SIGNAL(readyRead()), this, SLOT(readRequest()));

    QObject::connect(clientConnection, SIGNAL(disconnected()),
                     this, SLOT(on_disconnect()));
}

/**
	A Qt slot that is called every time a request is received from client.
	This method extracts URL from the request and loads the page associated 
	with that URL.
*/
void ServerParser::readRequest()
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

/**
	This method invokes the web-engine to load the page requested by client.
*/
void ServerParser::loadPage()
{
    webpage.settings()->clearMemoryCaches();
    webpage.settings()->clearIconDatabase();
    webpage.settings()->setAttribute(QWebSettings::JavascriptEnabled,true);
    diskCache->clear();
    webpage.mainFrame()->load(url);
}

/**
	A Qt slot that is called every time loading one object ends.
	The main responsibility of this method is doing some memory management
	and some logging tasks.
	
	@param reply An object provided by Qt framework that contains
	the information associated with the received HTTP response.
*/
void ServerParser::replyFinished(QNetworkReply * reply)
{
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

}

/**
	A Qt slot that is called every time the connection with client is disconnected.
*/
void ServerParser::on_disconnect()
{
    qDebug() << "--------- in disconnect ---------";
    clientConnection->deleteLater();
}

/**
	A Qt slot that is called every time the page load is finished.
	After page load ends, this method packs all the objects in one 
	bundle and sends it to the local proxy.
	
	@param status Indicates whether page load was successful
*/
void ServerParser::pageLoadFinished(bool status)
{
    webpage.settings()->setAttribute(QWebSettings::JavascriptEnabled,false);

    if(done)
        return;

     done = true;

    if (status)
    {       
        qDebug() << "----------Page load was successfull----------" ;
    }

    buildTar();
    sendBatch();
}

/**
	This method packs all the objects (contents of the "batch/" directory) 
	in one tar file.
*/
void ServerParser::buildTar()
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

/**
	This method sends the page bundle (batch.tar) to the client.
*/
void ServerParser::sendBatch()
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
