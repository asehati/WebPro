/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef CLIENT_H
#define CLIENT_H

#include "mynetworkaccessmanager.h"

#include <QTcpSocket>
#include <QFile>
#include <QDir>
#include <libtar.h>
#include <fcntl.h>
#include <QWebPage>
#include <QWebFrame>
#include <QtNetwork>
#include <QFileInfo>
#include <QFileInfoList>

QT_BEGIN_NAMESPACE
class QTcpSocket;
QT_END_NAMESPACE

//! [0]
class Client : public QObject
{
    Q_OBJECT

public:
    Client(QObject *parent = 0);
    void connectToServer();

    void setUrl(const QString &value);
    void setStatFileName(const QString &);

private slots:
    void displayError(QAbstractSocket::SocketError socketError);
    void readData();
    void objectLoadFinished(QNetworkReply *);
    void pageLoadFinished(bool k);

private:

    QTcpSocket *tcpSocket;
    quint32 blockSize;
    qint64 bytesWritten;
    QFile file; //batch files
    QFile statFile; // for writing timing measurements
    QTextStream out; //for writing to statFile
    QWebPage webpage;
    QString url;
    QString batchDir;
    QNetworkAccessManager* netAccess;
    QNetworkDiskCache *diskCache;
    QDataStream in;
    QSet<QUrl> cacheHit_urls;
    bool done;
    int objCounter;

    QTime timer;

    int secondBatchDownloadTime;

    void extractTar(const char *);
    void loadPage();
    void fileSetup(const char *);
    bool removeDir(const QString &);



};


#endif
