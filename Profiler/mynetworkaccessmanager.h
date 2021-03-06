#ifndef MYNETWORKACCESSMANAGER_H
#define MYNETWORKACCESSMANAGER_H

//uncomment in case you want to count the number of objects of different types
//#define COUNTSTATISTICS

#include <QNetworkAccessManager>
#include <QNetworkReply>

/**
    Class MyNetworkAccessManager
	
    A customized network access manager class. Implementing this class enables  
    profiler to prevent loading objects from cache and also to block adverisement 
    urls from being loaded and recorded in the resource file.
    
    @author Ali Sehati
    @version 1.0
*/
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

private slots:

};

#endif // MYNETWORKACCESSMANAGER_H
