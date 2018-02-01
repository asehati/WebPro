#ifndef NETWORKACCESSNULLER_H
#define NETWORKACCESSNULLER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>

class NetworkAccessNuller : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit NetworkAccessNuller(QObject *parent = 0);
    QNetworkReply * createRequest(Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0);

signals:

public slots:

};

#endif // NETWORKACCESSNULLER_H
