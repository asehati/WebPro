#include "mynetworkaccessmanager.h"

/**
    Class MyNetworkAccessManager
	
    A customized network access manager class. Implementing this class enables  
    remote proxy to prevent loading objects from cache and also to avoid loading 
    adverisement urls.
    
    @author Ali Sehati
    @version 1.0
*/
MyNetworkAccessManager::MyNetworkAccessManager(QObject *parent) :
    QNetworkAccessManager(parent)
{
}

/**
	Over-riding this method allows us to customize the HTTP request that will be issued 
	by the remote proxy. This customization includes preventing load from cache and also blocking Ad urls
*/
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
        myReq.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
        myReq.setAttribute(QNetworkRequest::CacheSaveControlAttribute, true);
        QNetworkReply * newResp = QNetworkAccessManager::createRequest(op, myReq, outgoingData);
        return newResp;
    }
}

/**
	Checks whether the provided url belongs to the black list of advertisement websites
	
	@param url url of the HTTP request to be issued by the remote proxy
	@return A boolean value indicating whether the url is for advertisement purposes
*/
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
