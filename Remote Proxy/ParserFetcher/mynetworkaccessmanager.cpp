#include "mynetworkaccessmanager.h"

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
        myReq.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);//QNetworkRequest::PreferCache
        myReq.setAttribute(QNetworkRequest::CacheSaveControlAttribute, true);
        QNetworkReply * newResp = QNetworkAccessManager::createRequest(op, myReq, outgoingData);
        return newResp;
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
        return true;
    }
    else
    {
        return false;
    }

}
