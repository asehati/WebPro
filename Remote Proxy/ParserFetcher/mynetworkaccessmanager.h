/**
    A customized network access manager class. Implementing this class enables  
    remote proxy to prevent loading objects from cache and also to avoid loading 
    adverisement urls.
    
    @author Ali Sehati
    @version 1.1
*/

#ifndef MYNETWORKACCESSMANAGER_H
#define MYNETWORKACCESSMANAGER_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>

class MyNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit MyNetworkAccessManager(QObject *parent = 0);
    QNetworkReply * createRequest(Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0);

private:
    bool isAdUrl(QUrl url);
signals:

public slots:

};

#endif // MYNETWORKACCESSMANAGER_H
