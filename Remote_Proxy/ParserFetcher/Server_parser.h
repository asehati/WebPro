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

class Server_parser : public QObject
{
    Q_OBJECT
public:
    explicit Server_parser(QObject *parent = 0);
    void setUrl(QString url);
    void loadPage();
    void startServer();
    void buildTar();
    void sendBatch();

private:
    int objCounter;
    QTcpServer *tcpServer;
    QTcpSocket *clientConnection;
    QWebPage webpage;
    QUrl url;
    QNetworkAccessManager* netAccess;
    QNetworkDiskCache *diskCache;
    bool done;
//    int reqNum;

signals:


private slots:
    void replyFinished(QNetworkReply * reply);
    void pageLoadFinished(bool k);
    void on_newConnection();
    void readRequest();
    void on_disconnect();

};

#endif // SERVER_PARSER_H
