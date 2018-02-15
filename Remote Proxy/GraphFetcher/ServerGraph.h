/**
    The server program which employs previously recorded resource lists to 
    load webpages requested by the local proxy. After page load ends,  
    the proxy packs all the objects in one bundle and sends it to 
    the local proxy.
    
    @author Ali Sehati
    @version 1.1
*/

#ifndef SERVER_GRAPH_H
#define SERVER_GRAPH_H

#include <libtar.h>
#include <fcntl.h>

#include <QObject>
#include <QWebPage>
#include <QWebFrame>
#include <QFile>
#include <QtNetwork>
#include <QDebug>
#include <QApplication>
#include <QTime>
#define PORT 8801



class ServerGraph : public QObject
{
    Q_OBJECT
public:
    explicit ServerGraph(QObject *parent = 0);
    void setUrl(QString url);
    void downloadObjects();
    void fetchObject(const QUrl &url);
    void buildTar(const char * batchName);
    void startServer();    
    void setRandListMissRatio(int value);
    void randListBuilder(int);

private:
    QTcpServer *tcpServer;
    QTcpSocket *clientConnection;
    QWebPage webpage;
    QUrl url;
    QNetworkAccessManager* netAccess;
    QNetworkAccessManager* netAccess_webEngine;
    QNetworkDiskCache *diskCache;
    QFile listFile;    
    QString listFilePath;
    QTextStream in;
    QList<QNetworkReply *> currentDownloads;
    QSet<int> randList;
    bool done;
    int reqNUm;
    int randListMissRatio;

    QString uniqueFileName(const QUrl &);
    void loadPage();
    void sendBatch();    

signals:

private slots:
    void downloadFinished(QNetworkReply * reply);
    void objectLoadFinished(QNetworkReply * reply);
    void pageLoadFinished(bool k);
    void sslErrors(const QList<QSslError> &);
    void on_newConnection();
    void readRequest();
    void on_disconnect();    

};

#endif // SERVER_PARSER_H
