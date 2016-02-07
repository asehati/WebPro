#include "networkaccessnuller.h"

NetworkAccessNuller::NetworkAccessNuller(QObject *parent) :
    QNetworkAccessManager(parent)
{
}

QNetworkReply *NetworkAccessNuller::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &req, QIODevice *outgoingData)
{
    QNetworkRequest myReq = QNetworkRequest(req);

    myReq.setAttribute(QNetworkRequest::CacheSaveControlAttribute, false);
    myReq.setUrl(QUrl());
    return QNetworkAccessManager::createRequest(QNetworkAccessManager::GetOperation, myReq);

}

