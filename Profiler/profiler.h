/**
	Profiler class which is used for visiting a webpage and \
	recording its resource list in a metadata repository
	
    @author Ali Sehati
    @version 1.1
*/

#ifndef PROFILER_H
#define PROFILER_H

#include "mynetworkaccessmanager.h"
#include <QApplication>
#include <QObject>
#include <QWebPage>
#include <QWebFrame>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtNetwork>
#include <QDebug>

class Profiler : public QObject
{
    Q_OBJECT

public:
    Profiler(QObject *parent = 0);
    void loadPage();

    void setUrl(const QString &);

private:
    QWebPage webpage;
    QString url;

    MyNetworkAccessManager *myNetAccess;
    QNetworkDiskCache *diskCache;

    QFile listFile;
    QTextStream out;

    QFile statFile;
    QTextStream statOut;

    QFile countFile;
    QTextStream countOut;

    int imageNum;
    int jsNum;
    int cssNum;
    int otherNum;

    bool done;
    QSet<QUrl> urlSet;
    QString dataDirectory;

    QString uniqueFileName(const QUrl &);
    void prepareLayout();
    void compareLists();
    void printResults();
    void checkType(QVariant);

private slots:
    void replyFinished(QNetworkReply * reply);
    void pageLoadFinished(bool k);

};

#endif // PROFILER_H
