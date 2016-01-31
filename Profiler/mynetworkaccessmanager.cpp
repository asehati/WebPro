#include "mynetworkaccessmanager.h"
#include "networkreplyproxy.h"

MyNetworkAccessManager::MyNetworkAccessManager(QObject *parent) :
    QNetworkAccessManager(parent)
{
}

QNetworkReply *MyNetworkAccessManager::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &req, QIODevice *outgoingData)
{
    QNetworkRequest myReq = QNetworkRequest(req);

    if (isAdUrl(req.url())){

//        myReq.setAttribute(QNetworkRequest::CacheSaveControlAttribute, false);
        myReq.setUrl(QUrl());

        return QNetworkAccessManager::createRequest(QNetworkAccessManager::GetOperation, myReq);
    }

#ifndef COUNTSTATISTICS
    else{

//        timeMap.insertMulti((req.url()).toString(),QTime::currentTime().msecsSinceStartOfDay());
//        qDebug() << (req.url()).toEncoded().constData() << "\t" << QTime::currentTime().msecsSinceStartOfDay();

        myReq.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);//QNetworkRequest::PreferCache
//        myReq.setAttribute(QNetworkRequest::CacheSaveControlAttribute, true);
        // sehati - july 20
         QNetworkReply* newRep = QNetworkAccessManager::createRequest(op, myReq, outgoingData);
//        QNetworkReply * newRep = QNetworkAccessManager::createRequest(op, myReq, outgoingData);
//        return new NetworkReplyProxy(this, newRep);
         //sehati - july 20

//         QObject::connect(newRep, SIGNAL(bytesWritten(qint64)),
//                          this, SLOT(requestWritten(qint64)));

//         QObject::connect(newRep, SIGNAL(finished()),
//                          this, SLOT(replyFinished()));
         return newRep;
   }
#else
    else
    {
        QNetworkReply * newRep = QNetworkAccessManager::createRequest(op, req, outgoingData);
        return new NetworkReplyProxy(this, newRep);
    }
#endif

}

bool MyNetworkAccessManager::isAdUrl(QUrl url)
{

   QString urlStr = url.toString();
//   qDebug() << "::" << urlStr;

    if(urlStr.contains("://ads.",Qt::CaseInsensitive) ||
       urlStr.contains("://ad.",Qt::CaseInsensitive) ||
       urlStr.contains("adsystem",Qt::CaseInsensitive) ||
       urlStr.contains("adserver.",Qt::CaseInsensitive) ||
       urlStr.contains(".doubleclick.",Qt::CaseInsensitive) ||
       urlStr.contains("google-analytics.com",Qt::CaseInsensitive) ||
       urlStr.contains("facebook.",Qt::CaseInsensitive) ||
       urlStr.contains("twitter.com",Qt::CaseInsensitive) ||
       urlStr.contains("moatads.com",Qt::CaseInsensitive) ||
       urlStr.contains("metrics.",Qt::CaseInsensitive) ||
       urlStr.contains("/ads/",Qt::CaseInsensitive) ||
       urlStr.contains("chartbeat.net",Qt::CaseInsensitive) ||
       urlStr.contains("scorecardresearch.com",Qt::CaseInsensitive) ||
       urlStr.contains("imrworldwide",Qt::CaseInsensitive) ||
       !urlStr.startsWith("http")
        )

    {       
        return true;
    }
    else
        return false;

}

/*
 * void MyNetworkAccessManager::requestWritten(qint64 s)
{
    QNetworkReply *senderReply = (QNetworkReply *)QObject::sender();// this is the object (qnetworkreply) that emitted this signal
    qDebug() << "*************************************:  " << senderReply->request().url();

}


void MyNetworkAccessManager::replyFinished()
{    
    QNetworkReply *senderReply = (QNetworkReply *)QObject::sender();// this is the object (qnetworkreply) that emitted this signal
//    qDebug() << "In replyFinished Slot:  " << senderReply->request().url().toString();
    timeMap.insertMulti(senderReply->request().url().toString(),QTime::currentTime().msecsSinceStartOfDay());
}

void MyNetworkAccessManager::printMap()
{
    QList<QString> keys = timeMap.uniqueKeys();

    for (int i = 0; i < keys.size(); ++i) {

        qDebug() << "key: " << keys[i];
        QList<int> values = timeMap.values(keys[i]);
        if (values.size() == 2)
            qDebug() << "Times: " << values.at(0) << "\t" << values.at(1) << "\t" << values.at(0) - values.at(1);
        else
        {
            qDebug() << "<<<<<<<<<<   >>>>>>>>>>>";
            qDebug() << values.size();
        }

    }
}
*/
