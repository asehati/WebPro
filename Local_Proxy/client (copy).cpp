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

#include <QtNetwork>

#include "client.h"

Client::Client(QObject *parent)
    :   QObject(parent)
{

    tcpSocket = new QTcpSocket(this);

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readData()));

    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));

    firstBatchDone = false;
    secondBatch = false;
    bytesWritten = 0;
    blockSize = 0;

    fileSetup("batch.tar");

    batchDir = QDir::currentPath();
    batchDir.append("/batch");

    if(!removeDir(batchDir))
        qDebug() << "Couldn't remove the batch directory";

    QObject::connect(&webpage, SIGNAL(loadFinished(bool)),
                     this, SLOT(pageLoadFinished(bool)));

    statFile.setFileName("statics.txt");
    statFile.open(QIODevice::Append);

    out.setDevice(&statFile);
    out.setRealNumberNotation(QTextStream::FixedNotation);
    out.setRealNumberPrecision(6);
}


void Client::connectToServer()
{
    QString request = "GET ";
    request.append(url);
    request.append(" HTTP 1.1\r\n\r\n");

    timer.start();

    tcpSocket->connectToHost("192.168.20.147",8801);
    if(tcpSocket->waitForConnected()) // putting 1 as parameter isn't reasonable, using default 3000ms value
    {
        tcpSocket->write(request.toUtf8().data());
        tcpSocket->flush();
    }
    else
    {
        qDebug() << "couldn't connect";
    }
}

void Client::readData()
{
    qint64 byteNum;

    if (blockSize == 0) {

        if (tcpSocket->bytesAvailable() < (int)sizeof(quint32))
            return;

        QByteArray bsize = tcpSocket->read((int)sizeof(quint32));
        QDataStream in(bsize);
        in.setVersion(QDataStream::Qt_4_0);
        in >> blockSize;
        qDebug() << "block size: " << blockSize;
    }

    byteNum = tcpSocket->bytesAvailable();

    QByteArray readData = tcpSocket->read(byteNum);

    file.write(readData);

    bytesWritten += byteNum;

    if (bytesWritten >= blockSize)
    {
        file.flush();
        file.close();

        if(!firstBatchDone)
        {
            extractTar("batch.tar");

            firstBatchDownloadTime = timer.elapsed();
            firstBatchDone = true;

            fileSetup("batch_miss.tar");
            blockSize = 0;
            bytesWritten = 0;
            loadPage();
        }
        else
        {
            qDebug() << "--- about to extract the second batch";

            extractTar("batch_miss.tar");
            secondBatchDownloadTime = timer.elapsed();

            //if (secondBatch)// maybe we receive the whole batch_miss file before the time secondBatch gets set to true
            if (blockSize > 0)
            {
                blockSize = 0;
                bytesWritten = 0;
                qDebug("Time elapsed in second batch: %f ms", (double)secondBatchDownloadTime / 1000);
                out << (float)secondBatchDownloadTime / 1000 << "\t";
                out.flush();
               // QCoreApplication::exit(0);
            }

            firstBatchDone = false;
        }
    }
}

void Client::objectLoadFinished(QNetworkReply *reply)
{
    QNetworkRequest req = reply->request();
    QUrl current_url = req.url();

    QVariant fromCache = reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute);

    qDebug() << current_url.toString() << " >>>>: " << fromCache.toBool();

    if(!fromCache.toBool())
        secondBatch = true;
}

void Client::pageLoadFinished(bool k)
{
    qDebug() << "--------------------- in page load finished -----------------------";
    if(!secondBatch)
    {
        qDebug("Time elapsed in page load: %f ms", (double)firstBatchDownloadTime / 1000);
        out << (float)firstBatchDownloadTime / 1000 << "\t";
        out.flush();
        QCoreApplication::exit(0);
    }
}


void Client::setUrl(const QString &value)
{
    url = value;
}


void Client::extractTar(const char * batchName)
{
    TAR *pTar;
    int errno;

    errno = tar_open(&pTar, batchName, NULL, O_RDONLY , 0644, TAR_GNU);
    if (  errno == -1 ){
        fprintf(stderr, "tar_open(): %s\n",strerror(errno));
        return;
    }

    errno = tar_extract_all(pTar,batchDir.toUtf8().data());
    if (errno != 0){
        fprintf(stderr,"tar_extract_all(): %s\n", strerror(errno));
        return;
    }

    errno = close(tar_fd(pTar));
    if ( errno != 0){
        fprintf(stderr,"tar_close(): %s\n",strerror(errno));
        return;
    }
}

void Client::loadPage()
{
    netAccess = new MyNetworkAccessManager(this);
    diskCache = new QNetworkDiskCache(this);
    diskCache->setCacheDirectory("batch");
    netAccess->setCache(diskCache);
    webpage.setNetworkAccessManager(netAccess);

    QObject::connect(netAccess, SIGNAL(finished(QNetworkReply *)),
                     this, SLOT(objectLoadFinished(QNetworkReply *)));

    qDebug() << ">>> " << this->url;
    webpage.mainFrame()->load(QUrl(url));
}


void Client::fileSetup(const char * batchName)
{
    file.setFileName(batchName);

    if(file.exists())
        file.remove();

    file.open(QIODevice::WriteOnly);
}

void Client::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        qDebug() << "Fortune Client The host was not found. Please check the host name and port settings.";
        break;
    case QAbstractSocket::ConnectionRefusedError:
        qDebug() << "Fortune Client The connection was refused by the peer. Make sure the fortune server is running, and check that the host name and port settings are correct.";
        break;
    default:
        qDebug() << "Fortune Client The following error occurred:" << tcpSocket->errorString();
    }

}


bool Client::removeDir(const QString &dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName))
    {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            }
            else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }

    return result;
}

