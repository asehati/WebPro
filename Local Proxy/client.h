#ifndef CLIENT_H
#define CLIENT_H

#include "mynetworkaccessmanager.h"

#include <QTcpSocket>
#include <QFile>
#include <QDir>
#include <libtar.h>
#include <fcntl.h>
#include <QWebPage>
#include <QWebFrame>
#include <QtNetwork>
#include <QFileInfo>
#include <QFileInfoList>

QT_BEGIN_NAMESPACE
class QTcpSocket;
QT_END_NAMESPACE

/**
    Class Client
	
    The client program which implements local proxy.
    It forwards the first request to remote proxy, waits to receive the bundle from remote proxy,
    then serves all the subsequent requests from objects in the bundle. Missing objects in the bundle,
    are fetched directly from remote web-servers.
    
    @author Ali Sehati
    @version 1.0
*/
class Client : public QObject
{
    Q_OBJECT

public:
    Client(QObject *parent = 0);
    void connectToServer();

    void setUrl(const QString &value);
    void setStatFileName(const QString &);

private slots:
    void displayError(QAbstractSocket::SocketError socketError);
    void readData();
    void objectLoadFinished(QNetworkReply *);
    void pageLoadFinished(bool k);

private:

    QTcpSocket *tcpSocket;
    quint32 blockSize;
    qint64 bytesWritten;
    QFile file; //batch files
    QFile statFile; // for writing timing measurements
    QTextStream out; //for writing to statFile
    QWebPage webpage;
    QString url;
    QString batchDir;
    QNetworkAccessManager* netAccess;
    QNetworkDiskCache *diskCache;
    QDataStream in;
    QSet<QUrl> cacheHit_urls;
    bool done;

    QTime timer;

    int secondBatchDownloadTime;

    void extractTar(const char *);
    void loadPage();
    void fileSetup(const char *);
    bool removeDir(const QString &);

};

#endif
