#include "mynetworkaccessmanager.h"
//#include "networkreplyproxy.h"

MyNetworkAccessManager::MyNetworkAccessManager(QObject *parent) :
    QNetworkAccessManager(parent)
{
}

QNetworkReply *MyNetworkAccessManager::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &req, QIODevice *outgoingData)
{
    QNetworkRequest myReq = QNetworkRequest(req);

    if (isAdUrl(req.url()))
    {
        myReq.setAttribute(QNetworkRequest::CacheSaveControlAttribute, false);
        myReq.setUrl(QUrl());
        return QNetworkAccessManager::createRequest(QNetworkAccessManager::GetOperation, myReq);
    }
    else
    {
        myReq.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
        myReq.setAttribute(QNetworkRequest::CacheSaveControlAttribute, true);
//        myReq.setHeader(QNetworkRequest::UserAgentHeader,"Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:30.0) Gecko/20100101 Firefox/30.0");
        QNetworkReply * newRep = QNetworkAccessManager::createRequest(op, myReq, outgoingData);
        return newRep;
    }
}

bool MyNetworkAccessManager::isAdUrl(QUrl url)
{

    QString urlStr = url.toString();

    if(urlStr.contains("://ads.",Qt::CaseInsensitive) ||
       urlStr.contains("://ad.",Qt::CaseInsensitive) ||
       urlStr.contains("adsystem",Qt::CaseInsensitive) ||
       urlStr.contains("adserver.",Qt::CaseInsensitive) ||
       urlStr.contains(".doubleclick.",Qt::CaseInsensitive) ||
       urlStr.contains("google-analytics.com",Qt::CaseInsensitive) ||
       urlStr.contains("facebook.com",Qt::CaseInsensitive) ||
       urlStr.contains("twitter.com",Qt::CaseInsensitive) ||
       urlStr.contains("moatads.com",Qt::CaseInsensitive) ||
       urlStr.contains("metrics.",Qt::CaseInsensitive) ||
       urlStr.contains("/ads/",Qt::CaseInsensitive) ||
       urlStr.contains("chartbeat.net",Qt::CaseInsensitive) ||
       urlStr.contains("scorecardresearch.com",Qt::CaseInsensitive) ||
       urlStr.contains("imrworldwide",Qt::CaseInsensitive) ||
       !urlStr.startsWith("http"))

    {
//        qDebug() << "  -------  " << url.toString();
        return true;
    }
    else
        return false;

}
