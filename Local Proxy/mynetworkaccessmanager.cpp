/**
    A customized network access manager class. Implementing this class enables  
    local proxy to serve most objects from cache. Those objects that are missing
	in the bundle (cache) will be fetched directly from remote servers.
    Implementing this class also enables proxy to avoid loading advertisement urls
	
    @author Ali Sehati
    @version 1.1
*/

#include "myNetworkAccessManager.h"

MyNetworkAccessManager::MyNetworkAccessManager(QObject *parent) :
    QNetworkAccessManager(parent)
{
}

/**
	Over-riding this method allows us to customize the HTTP request that will be issued 
	by the local proxy. This customization includes prefering load from cache and also blocking Ad urls
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
        myReq.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
        myReq.setAttribute(QNetworkRequest::CacheSaveControlAttribute, true);
        QNetworkReply * newRep = QNetworkAccessManager::createRequest(op, myReq, outgoingData);
        return newRep;
    }
}

/**
	Checks whether the provided url belongs to the black list of advertisement websites
	
	@param url url of the HTTP request to be issued by the local proxy
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
