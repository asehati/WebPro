#ifndef MYNETWORKACCESSMANAGER_H
#define MYNETWORKACCESSMANAGER_H

//#define COUNTSTATISTICS

#include <QNetworkAccessManager>
#include <QNetworkReply>

class MyNetworkAccessManager : public QNetworkAccessManager
{

    Q_OBJECT
public:
    explicit MyNetworkAccessManager(QObject *parent = 0);
    QNetworkReply * createRequest(Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0);    
//    void printMap();

private:
//    QMap<QString, int> timeMap;
    bool isAdUrl(QUrl url);

signals:

public slots:

private slots:
//    void requestWritten(qint64 s);
//    void replyFinished();

};

#endif // MYNETWORKACCESSMANAGER_H
