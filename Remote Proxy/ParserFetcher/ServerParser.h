#ifndef SERVER_PARSER_H
#define SERVER_PARSER_H

#include <libtar.h>
#include <fcntl.h>

#include <QObject>
#include <QWebPage>
#include <QWebFrame>
#include <QFile>
#include <QtNetwork>
#include <QDebug>
#define PORT 8801

/**
    Class ServerParser
	
    The server program which employs a web-engine to load webpages
    requested by the local proxy. After page load ends, the proxy packs all 
    the objects in one bundle and sends it to the local proxy.
    
    @author Ali Sehati
    @version 1.0
*/
class ServerParser : public QObject
{
    Q_OBJECT
public:
    explicit ServerParser(QObject *parent = 0);
    void setUrl(QString url);
    void loadPage();
    void startServer();
    void buildTar();
    void sendBatch();

private:
    QTcpServer *tcpServer;
    QTcpSocket *clientConnection;
    QWebPage webpage;
    QUrl url;
    QNetworkAccessManager* netAccess;
    QNetworkDiskCache *diskCache;
    bool done;

signals:


private slots:
    void replyFinished(QNetworkReply * reply);
    void pageLoadFinished(bool k);
    void on_newConnection();
    void readRequest();
    void on_disconnect();

};

#endif // SERVER_PARSER_H
