/**
    The server program which employs previously recorded resource lists to 
    load webpages requested by the local proxy. After page load ends,  
    the proxy packs all the objects in one bundle and sends it to 
    the local proxy.
    
    @author Ali Sehati
    @version 1.1
*/

#include "ServerGraph.h"
#include "mynetworkaccessmanager.h"
#include <QNetworkCacheMetaData>
#define high 20

ServerGraph::ServerGraph(QObject *parent) :
    QObject(parent)
{
    reqNUm = 0;

    diskCache = new QNetworkDiskCache(this);
    diskCache->setCacheDirectory("batch");
    diskCache->clear();

    netAccess = new QNetworkAccessManager();
    netAccess->setCache(diskCache);
    QObject::connect(netAccess, SIGNAL(finished(QNetworkReply *)),
                     this, SLOT(downloadFinished(QNetworkReply *)));

    netAccess_webEngine = new MyNetworkAccessManager(this);
    netAccess_webEngine->setCache(diskCache);
    webpage.setNetworkAccessManager(netAccess_webEngine);

    QObject::connect(netAccess_webEngine, SIGNAL(finished(QNetworkReply *)),
                     this, SLOT(objectLoadFinished(QNetworkReply *)));

    QObject::connect(&webpage, SIGNAL(loadFinished(bool)),
                     this, SLOT(pageLoadFinished(bool)));

    webpage.settings()->clearMemoryCaches();
}

/**
	This method starts the server. The server listens at port PORT 
	for requests coming from the client.
*/
void ServerGraph::startServer()
{
	const QString IP_ADDRESS = "136.159.7.120";
    tcpServer = new QTcpServer(this);
    QObject::connect(tcpServer, SIGNAL(newConnection()), this, SLOT(on_newConnection()));

    if(!tcpServer->listen(QHostAddress(IP_ADDRESS),PORT))
        qDebug() << "Could not listen to port: " << PORT;
}

/**
	A Qt slot that is called every time a new connection establishment  
	request is received from client. In case this request number is greater 
	than 20, we start counting request numbers from 1. Also, in that case
	we generate a new random list. This provision is to support scenarios
	where remote proxy is used to serve a large number of requests 
	back-to-back.
*/
void ServerGraph::on_newConnection()
{
    clientConnection = tcpServer->nextPendingConnection();
	
    if(clientConnection->state() != QTcpSocket::ConnectedState)
    {
        qDebug() << "-----New connection is not in connected state------";
    }

    done = false;
	
#ifdef MIX_APPROACH
    qDebug() << "----------- using Mixed approach-------------";
    reqNUm++;

    if(reqNUm > high)
    {
        reqNUm = 1;
        randList.clear();
        randListBuilder(randListMissRatio);
    }
#endif

    QObject::connect(clientConnection, SIGNAL(readyRead()), this, SLOT(readRequest()));
    QObject::connect(clientConnection, SIGNAL(disconnected()),
                     this, SLOT(on_disconnect()));
}

/**
	A Qt slot that is called every time the connection with client is disconnected.
*/
void ServerGraph::on_disconnect()
{
    qDebug() << "--------- in disconnect ---------";
    clientConnection->deleteLater();
    webpage.settings()->clearMemoryCaches();
    webpage.settings()->clearIconDatabase();
}

/**
	A Qt slot that is called every time a request is received from client.
	This method extracts URL from the request and loads the page associated 
	with that URL. In case the request number belongs to random list, then 
	page will be loaded using web-engine. Otherwise, it will be loaded using 
	previously recorded resource list.
*/
void ServerGraph::readRequest()
{
    if (clientConnection->canReadLine())
    {
        QStringList tokens = QString(clientConnection->readLine()).split(QRegExp("[ \r\n][ \r\n]*"));
        if (tokens[0] == "GET")
        {
            setUrl(tokens[1]);
            listFilePath = QString("profiles/") + uniqueFileName(this->url);
            QFile file(listFilePath);

#ifdef MIX_APPROACH
            if(!randList.contains(reqNUm))
#else
            if(file.exists())
#endif
            {
                downloadObjects();
            }
            else
            {
                loadPage();
            }

        }
    }
}

/**
	This method invokes the web-engine to load the page requested by client.
*/
void ServerGraph::loadPage()
{
    webpage.settings()->clearMemoryCaches();
    webpage.settings()->clearIconDatabase();
    webpage.settings()->setAttribute(QWebSettings::JavascriptEnabled,true);
    diskCache->clear();
    webpage.mainFrame()->load(url);
}

/**
	This method sets the url that was embedded in client's request.
	
	@param myUrl The url associated with client requested page
*/
void ServerGraph::setUrl(QString myUrl)
{
    url.setUrl(myUrl);
}

/**
	This method downloads all the objects in the resource list of 
	the webpage referenced by url.
*/
void ServerGraph::downloadObjects()
{    	
    listFile.setFileName(listFilePath);
    in.setDevice(&listFile);

    if (!listFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "-------Could not open resource list file------";
        return;
    }

    while (!in.atEnd()) {
        QString line = in.readLine();
        QUrl url = QUrl(line);
        fetchObject(url);
    }

    listFile.close();
}

/**
	This method downloads the object referenced by url. 
	
	@param url URL of the object to be fetched
*/
void ServerGraph::fetchObject(const QUrl &url)
{
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    request.setAttribute(QNetworkRequest::CacheSaveControlAttribute, true);
    QNetworkReply *reply = netAccess->get(request);

#ifndef QT_NO_SSL
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)), SLOT(sslErrors(QList<QSslError>)));
#endif

    currentDownloads.append(reply);
}

/**
	A Qt slot that is called every time there is an SSL error.
*/
void ServerGraph::sslErrors(const QList<QSslError> &sslErrors)
{
#ifndef QT_NO_SSL
    foreach (const QSslError &error, sslErrors)
        fprintf(stderr, "SSL error: %s\n", qPrintable(error.errorString()));
#else
    Q_UNUSED(sslErrors);
#endif
}

/**
	A Qt slot that is called every time loading one object from resource list
	is ended. This method checks to see if this was the last object remaining
	from resource list. In that case, it calls specific methods to create a 
	page bundle from downloaded objects and sends it to the client.
	
	@param reply An object provided by Qt framework that contains
	the information associated with the received HTTP response.
*/
void ServerGraph::downloadFinished(QNetworkReply *reply)
{
    QUrl url = reply->url();

    if (reply->error()) {
        fprintf(stderr, "Download of %s failed: %s\n",
                url.toEncoded().constData(),
                qPrintable(reply->errorString()));
    }

    currentDownloads.removeAll(reply);
    reply->deleteLater();

    if (currentDownloads.isEmpty()) // all downloads finished
    {
        buildTar("batch.tar"); 

        sendBatch();

        diskCache->clear();
    }
}

/**
	A Qt slot associated with web-engine that is called every time loading 
	one object ends in the web-engine. The main responsibility of this method 
	is doing some memory management and some logging tasks.
	
	@param reply An object provided by Qt framework that contains
	the information associated with the received HTTP response.
*/
void ServerGraph::objectLoadFinished(QNetworkReply *reply)
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
	A Qt slot that is called every time the page load using web-engine
	is finished. After page load ends, this method packs all the objects 
	in one bundle and sends it to the local proxy.
	
	@param status Indicates whether page load was successful
*/
void ServerGraph::pageLoadFinished(bool status)
{
    webpage.settings()->setAttribute(QWebSettings::JavascriptEnabled,false);

    if(done)
        return;

    done = true;

    if (status)
    {
        qDebug() << "-----------page loaded successfully---------------";
    }
    else
	{
        qDebug() << "-----------A problem in loading page---------------";
	}


    buildTar("batch.tar"); 

    sendBatch();

    diskCache->clear(); 
}

/**
	This method packs all the objects (contents of the "batch/" directory) 
	in one tar file.
	
	@param batchName Name of the tar file to be created (page bundle)
*/
void ServerGraph::buildTar(const char * batchName)
{
    QFile file(batchName);

    if(file.exists())
        file.remove();

    TAR *pTar;

    tar_open(&pTar, batchName, NULL, O_WRONLY | O_CREAT, 0644, TAR_GNU);

    tar_append_tree(pTar, "batch/", ".");

    tar_close(pTar);
}

/**
	This method sends the page bundle (batch.tar) to the client.
*/
void ServerGraph::sendBatch()
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

/**
	A method to generate a unique file name from a given URL
	
	@param url the url to visit and create its resource file
	@return name of the file that will hold the resource list of this url
*/
QString ServerGraph::uniqueFileName(const QUrl &url)
{
    QUrl cleanUrl = url;
    cleanUrl.setPassword(QString());
    cleanUrl.setFragment(QString());

    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(cleanUrl.toEncoded());
	
    // convert sha1 to base36 form and return first 8 bytes for use as string
    QByteArray id =  QByteArray::number(*(qlonglong*)hash.result().data(), 36).left(8);
	
    // generates <one-char subdir>/<8-char filname.d>
    uint code = (uint)id.at(id.length()-1) % 16;
    QString pathFragment = QString::number(code, 16) + QLatin1Char('/')
            + QLatin1String(id);

    return pathFragment;
}

/**
	This method creates a set of random numbers and stores them in
	a list called randList. The generated random numbers specify
	request numbers that will be served using a web-engine.
	
	@param missNumbers Number of requests to be served using web-engine
*/
void ServerGraph::randListBuilder(int missNumbers)
{
    int count = 0;
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());

    while(count < missNumbers)
    {        
        int randNum = qrand() % ((high + 1) - 1) + 1;

        if(!randList.contains(randNum))
            count++;

        randList.insert(randNum);
    }

}

/**
	Sets the number of requests that should be served using web-engine
	instead of resource lists. 
	
	@param value The number of requests to be served using a web-engine
*/
void ServerGraph::setRandListMissRatio(int value)
{
    randListMissRatio = value;
}

