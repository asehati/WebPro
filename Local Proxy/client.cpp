/**
    The client program which implements local proxy.
    It forwards the first request to remote proxy, waits to receive the bundle from remote proxy,
    then serves all the subsequent requests from objects in the bundle. Missing objects in the bundle,
    are fetched directly from remote web-servers.
    
    @author Ali Sehati
    @version 1.1
*/

#include <QtNetwork>

#include "client.h"

Client::Client(QObject *parent)
    :   QObject(parent)
{

    tcpSocket = new QTcpSocket(this);
    in.setDevice(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readData()));

    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));

    done = false;
    bytesWritten = blockSize = 0;

    cacheHit_urls.clear();

    fileSetup("batch.tar");

    batchDir = QDir::currentPath();
    batchDir.append("/batch");

    if(!removeDir(batchDir))
        qDebug() << "Couldn't remove the batch directory";

    netAccess = new MyNetworkAccessManager(this);
    diskCache = new QNetworkDiskCache(this);
    diskCache->setCacheDirectory("batch");
    netAccess->setCache(diskCache);
    webpage.setNetworkAccessManager(netAccess);

    QObject::connect(netAccess, SIGNAL(finished(QNetworkReply *)),
                     this, SLOT(objectLoadFinished(QNetworkReply *)));

    QObject::connect(&webpage, SIGNAL(loadFinished(bool)),
                     this, SLOT(pageLoadFinished(bool)));

    out.setDevice(&statFile);
    out.setRealNumberNotation(QTextStream::FixedNotation);
    out.setRealNumberPrecision(6);
}

/**
	Sets the url that the browser wants to visit
	
	@param value the url to be visited by the browser
*/
void Client::setUrl(const QString &value)
{
    url = value;
}

/**
	Set and open the log file that is going to hold timing information
	
	@param value name of the log file
*/
void Client::setStatFileName(const QString &value)
{
    statFile.setFileName(value);
    statFile.open(QIODevice::Append);
}

/**
	Creates an HTTP request with the user-requested url and sends it to the remote proxy
*/
void Client::connectToServer()
{	 
	const QString IP_ADDRESS = "136.159.7.120"; //public IP address of the remote proxy
	const int PORT = 8801;	//port number where proxy process is listening
	
    QString request = "GET ";
    request.append(url);
    request.append(" HTTP 1.1\r\n\r\n");

    timer.start();

    tcpSocket->connectToHost(IP_ADDRESS,PORT); 
	
    if(tcpSocket->waitForConnected()) // putting 1 as parameter isn't reasonable, using default 3000ms value
    {
        tcpSocket->write(request.toUtf8().data());
        tcpSocket->flush();
    }
    else
    {
        qDebug() << "Couldn't connect to server.";
    }
}

/**
	A Qt slot that is called every time a data is received from remote proxy.
	The first 4 bytes indicate the size of the bundle.
	After receiving the entire bundle, it extracts the bundle and invokes 
	web engine to load the page
*/
void Client::readData()
{

    if (blockSize == 0) {

        if (tcpSocket->bytesAvailable() < (int)sizeof(quint32))
            return;

        in >> blockSize;
        qDebug() << "block size: " << blockSize;
    }

    int bytesToRead = std::min(tcpSocket->bytesAvailable(),blockSize - bytesWritten);
    QByteArray readData = tcpSocket->read(bytesToRead);

    file.write(readData);

    bytesWritten += bytesToRead;

    if (bytesWritten >= blockSize)
    {
        file.flush();
        file.close();

        extractTar("batch.tar");

        blockSize = 0;
        bytesWritten = 0;

        loadPage();
    }
}

/**
	A Qt slot that is called every time the HTTP response for one object is received.
	The program takes the URL of the received object and records it in the associated resource file
	
	@param reply contains the information associated with the completed HTTP request-response transaction
*/
void Client::objectLoadFinished(QNetworkReply *reply)
{
    objCounter++;

    if(done)
        return;

    QUrl current_url = reply->request().url();

    reply->deleteLater();

    if(current_url.isEmpty())
        return;

    QVariant fromCache = reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute);

    if(fromCache.toBool())
    {
        cacheHit_urls.insert(current_url);
    }
}

/**
	A Qt slot that is called every time the entire page load is finished
	The call of this function also triggers stopping the timer that measures 
	page load time. The time value is written to a log file
	
	@param status Indicates whether page load process was successful
*/
void Client::pageLoadFinished(bool status)
{
    webpage.settings()->setAttribute(QWebSettings::JavascriptEnabled,false);

    if(done)
        return;

    done = true;
    int loadTime = timer.elapsed();
   
    qDebug("Time elapsed in page load: %f ms", (double)loadTime / 1000);
    qDebug() << "Cache Hit #: " << cacheHit_urls.size();
    out << (float)loadTime / 1000 << "\t";
    out.flush();

    QCoreApplication::exit(0);
}


/**
	Extracts the tar file that holds the bundle received from remote proxy.
	
	@param batchName Name of the tar file that contains the bundle
*/
void Client::extractTar(const char * batchName)
{
    TAR *pTar;
    int errno;

    errno = tar_open(&pTar, batchName, NULL, O_RDONLY , 0644, TAR_GNU);
    if (  errno == -1 ){
        fprintf(stderr, "tar_open(): %s\n",strerror(errno));
        return;
    }

    errno = tar_extract_all(pTar,batchDir.toUtf8().data());
    if (errno != 0){
        fprintf(stderr,"tar_extract_all(): %s\n", strerror(errno));
        return;
    }

    errno = tar_close(pTar);
    if ( errno != 0){
        fprintf(stderr,"tar_close(): %s\n",strerror(errno));
        return;
    }
}

/**
	Starts loading the page associated with url at the client. Most objects will be served 
	from the received bundle. The ones missing in the bundle will be fetched from remote
	web servers.
*/
void Client::loadPage()
{
    webpage.settings()->clearMemoryCaches();
    webpage.settings()->clearIconDatabase();    

    qDebug() << ">>> " << this->url;
    webpage.mainFrame()->load(QUrl(url));
}

/**
	Sets up and opens the batch file that will be used to write the received bundle from remote proxy
	"file" is the handle that will be used to work with this file
	
	@param batchName Name of the batch file that will hold the received page bundle
*/
void Client::fileSetup(const char * batchName)
{
    file.setFileName(batchName);

    if(file.exists())
        file.remove();

    file.open(QIODevice::WriteOnly);
}

/**
	A Qt slot that is called in case there is an error in establishing a connection to the server.
	
	@param socketError parameter passed by the Qt framework that specifies the type of the error occured
*/
void Client::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        qDebug() << "Client Proxy: The host was not found. Please check the host name and port settings.";
        break;
    case QAbstractSocket::ConnectionRefusedError:
        qDebug() << "Client Proxy: The connection was refused by the peer. Make sure the GraphFetcher server is running, and check that the host name and port settings are correct.";
        break;
    default:
        qDebug() << "Client Proxy: The following error occurred:" << tcpSocket->errorString();
    }

}


/**
	Removes the entire directory which contains the contents of the received page bundle.
	This directory contains multiple subdirectories where the actual web objects reside.
	
	@param dirName Absolute path for the directory that holds the contents of the bundle.
	@return Whether remove directory was performed successfully
*/
bool Client::removeDir(const QString &dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName))
    {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            }
            else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }

    return result;
}

