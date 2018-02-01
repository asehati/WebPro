#include "Server_graph.h"
#include "mynetworkaccessmanager.h"
#include "networkaccessnuller.h"
#include <QNetworkCacheMetaData>
#define high 20

Server_graph::Server_graph(QObject *parent) :
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

    //    webpage.settings()->setIconDatabasePath(""); //disable icon database for favicons associated with websites

}

void Server_graph::on_disconnect()
{
    qDebug() << "--------- in disconnect ---------";
    clientConnection->deleteLater();
    webpage.settings()->clearMemoryCaches();
    webpage.settings()->clearIconDatabase();
}

void Server_graph::sslErrors(const QList<QSslError> &sslErrors)
{
#ifndef QT_NO_SSL
    foreach (const QSslError &error, sslErrors)
        fprintf(stderr, "SSL error: %s\n", qPrintable(error.errorString()));
#else
    Q_UNUSED(sslErrors);
#endif
}

void Server_graph::on_newConnection()
{
    qDebug() << "A new client connected to server";
    clientConnection = tcpServer->nextPendingConnection();
    if(clientConnection->state() != QTcpSocket::ConnectedState)
    {
        qDebug() << "New connection is not in connected state.";
    }

    done = false;
#ifdef MIX_APPROACH
    qDebug() << "------------------- using mixture approach";
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


void Server_graph::readRequest()
{
    if (clientConnection->canReadLine())
    {
        QStringList tokens = QString(clientConnection->readLine()).split(QRegExp("[ \r\n][ \r\n]*"));
        if (tokens[0] == "GET")
        {
            qDebug() << " ---> From Client: " << tokens[1];
            setUrl(tokens[1]);

            listFilePath = QString("profiles/") + uniqueFileName(this->url);
            qDebug() << "filePath: " << listFilePath;
            QFile file(listFilePath);

            //the commented if statement represents the real implementation that should be used
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
                qDebug() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> url: " << tokens[1];
                loadPage();
            }

        }
    }
}

void Server_graph::loadPage()
{
    webpage.settings()->clearMemoryCaches();
    webpage.settings()->clearIconDatabase();
    webpage.settings()->setAttribute(QWebSettings::JavascriptEnabled,true);
    diskCache->clear();
    webpage.mainFrame()->load(url);
}

void Server_graph::setUrl(QString myUrl)
{
    url.setUrl(myUrl);
}

void Server_graph::downloadObjects()
{    
    listFile.setFileName(listFilePath);
    in.setDevice(&listFile);

    if (!listFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Could not open urlList.txt";
        return;
    }

    GraphSize = 0;

    while (!in.atEnd()) {
        GraphSize++;
        QString line = in.readLine();
        QUrl url = QUrl(line);
        fetchObject(url);
    }

    listFile.close();
}

void Server_graph::fetchObject(const QUrl &url)
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

//downloading all the objects in the resource graph
void Server_graph::downloadFinished(QNetworkReply *reply)
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
        buildTar("batch.tar"); // build the first batch of objects

        qDebug() << "-- in download finished after build tar";

        sendBatch();

        qDebug() << "Graph Size: " << GraphSize;
        diskCache->clear();
    }
}


void Server_graph::objectLoadFinished(QNetworkReply *reply)
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

//page load using cached objects -> for detecting missed objects
void Server_graph::pageLoadFinished(bool k)
{
    webpage.settings()->setAttribute(QWebSettings::JavascriptEnabled,false);

    if(done)
        return;

    done = true;

    if (k)
    {
        qDebug() << "-----------page loaded successfully---------------";
    }
    else
        qDebug() << "-----------A problem in loading page---------------";


    buildTar("batch.tar"); // build the first batch of objects

    sendBatch();

    diskCache->clear(); // could be in on_disconnected
}

void Server_graph::buildTar(const char * batchName)
{
//    timer.start();

    QFile file(batchName);

    if(file.exists())
        file.remove();

    TAR *pTar;

    tar_open(&pTar, batchName, NULL, O_WRONLY | O_CREAT, 0644, TAR_GNU);

    tar_append_tree(pTar, "batch/", ".");

    tar_close(pTar);

//    int buildTarTime = timer.elapsed();
//    qDebug() << "++++++++++++++ Time elapsed in building tar: " << buildTarTime;

}

void Server_graph::sendBatch()
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

QString Server_graph::uniqueFileName(const QUrl &url)
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

void Server_graph::startServer()
{
    tcpServer = new QTcpServer(this);
    QObject::connect(tcpServer, SIGNAL(newConnection()), this, SLOT(on_newConnection()));

    if(!tcpServer->listen(QHostAddress("136.159.7.120"),PORT))
        qDebug() << "Could not listen to port: " << PORT;
}

void Server_graph::randListBuilder(int missNumbers)
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

//    QSet<int>::ConstIterator it = randList.constBegin(),end = randList.constEnd();

//    for (; it != end; ++it) {
//        qDebug() << "rand: " << *it;
//    }

}

void Server_graph::setRandListMissRatio(int value)
{
    randListMissRatio = value;
}

