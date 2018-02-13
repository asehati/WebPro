/**
    A customized network access manager class. Implementing this class enables  
    local proxy to serve most objects from cache. Those objects that are missing
	in the bundle (cache) will be fetched directly from remote servers.
    Implementing this class also enables proxy to avoid loading advertisement urls
	
    @author Ali Sehati
    @version 1.1
*/

#ifndef MYNETWORKACCESSMANAGER_H
#define MYNETWORKACCESSMANAGER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>

class MyNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit MyNetworkAccessManager(QObject *parent = 0);
    QNetworkReply * createRequest(Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0);

private:
    bool isAdUrl(QUrl);

signals:

public slots:

};

#endif // MYNETWORKACCESSMANAGER_H
