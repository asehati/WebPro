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
    in.setDevice(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readData()));

    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));

    done = false;
    bytesWritten = 0;
    blockSize = 0;
    objCounter = 0;

    cacheHit_urls.clear();

    fileSetup("batch.tar");

    batchDir = QDir::currentPath();
    batchDir.append("/batch");

    if(!removeDir(batchDir))
        qDebug() << "Couldn't remove the batch directory";

    netAccess = new MyNetworkAccessManager(this);
    diskCache = new QNetworkDiskCache(this);
    diskCache->setCacheDirectory("batch");
    netAccess->setCache(diskCache);
    webpage.setNetworkAccessManager(netAccess);

    QObject::connect(netAccess, SIGNAL(finished(QNetworkReply *)),
                     this, SLOT(objectLoadFinished(QNetworkReply *)));

    QObject::connect(&webpage, SIGNAL(loadFinished(bool)),
                     this, SLOT(pageLoadFinished(bool)));

    out.setDevice(&statFile);
    out.setRealNumberNotation(QTextStream::FixedNotation);
    out.setRealNumberPrecision(6);
}

void Client::setStatFileName(const QString &value)
{
    statFile.setFileName(value);
    statFile.open(QIODevice::Append);
}


void Client::connectToServer()
{
    QString request = "GET ";
    request.append(url);
    request.append(" HTTP 1.1\r\n\r\n");

    timer.start();

    tcpSocket->connectToHost("136.159.7.120",8801); //"192.168.20.147"
    if(tcpSocket->waitForConnected()) // putting 1 as parameter isn't reasonable, using default 3000ms value
    {
        tcpSocket->write(request.toUtf8().data());
        tcpSocket->flush();
    }
    else
    {
        qDebug() << "Couldn't connect to server.";
    }
}

void Client::readData()
{

    if (blockSize == 0) {

        if (tcpSocket->bytesAvailable() < (int)sizeof(quint32))
            return;

        in >> blockSize;
        qDebug() << "block size: " << blockSize;
    }

    int bytesToRead = std::min(tcpSocket->bytesAvailable(),blockSize - bytesWritten);
    QByteArray readData = tcpSocket->read(bytesToRead);

    file.write(readData);

    bytesWritten += bytesToRead;

    if (bytesWritten >= blockSize)
    {
        file.flush();
        file.close();

        extractTar("batch.tar");

        blockSize = 0;
        bytesWritten = 0;

        loadPage();
    }
}


void Client::objectLoadFinished(QNetworkReply *reply)
{
    objCounter++;

//    if(done)
//        return;

    QUrl current_url = reply->request().url();

    reply->deleteLater();

    if(current_url.isEmpty())
        return;

    QVariant fromCache = reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute);

//    QVariant isRevalid = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
//    if(isRevalid.isValid())
//    {
//        int status = isRevalid.toInt();
//        QString reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
//        qDebug() << "Status:  " << status << "  reason:  " << reason;
//    }

    if(fromCache.toBool())
    {
        cacheHit_urls.insert(current_url);
    }

    if(objCounter == 5)
    {
        int loadTime = timer.elapsed();
        qDebug("Time elapsed in page load: %f ms", (double)loadTime / 1000);
        qDebug() << "Cache Hit #: " << cacheHit_urls.size();
//        out << (float)loadTime / 1000 << "\t";
         out << (float)loadTime << "\t";
        out.flush();

        statFile.close();

        QCoreApplication::exit(0);
    }
}

void Client::pageLoadFinished(bool k)
{
    if(k)
        qDebug() << "--------------------- in page load finished -----------------------";
}

/*void Client::pageLoadFinished(bool k)
{
    webpage.settings()->setAttribute(QWebSettings::JavascriptEnabled,false);

    if(done)
        return;

//    timeOut.stop();

    done = true;
    int loadTime = timer.elapsed();
    qDebug() << "--------------------- in page load finished -----------------------";
    qDebug("Time elapsed in page load: %f ms", (double)loadTime / 1000);
    qDebug() << "Cache Hit #: " << cacheHit_urls.size();
    out << (float)loadTime / 1000 << "\t";
    out.flush();

//    qDebug() << "++++++++++++++++++++++++++++++++++++++++";
//    qDebug() << webpage.mainFrame()->toPlainText();

    QCoreApplication::exit(0);
}*/


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

//    errno = close(tar_fd(pTar));
    errno = tar_close(pTar);
    if ( errno != 0){
        fprintf(stderr,"tar_close(): %s\n",strerror(errno));
        return;
    }
}

void Client::loadPage()
{
    webpage.settings()->clearMemoryCaches();
    webpage.settings()->clearIconDatabase();    

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
        qDebug() << "Client Proxy: The host was not found. Please check the host name and port settings.";
        break;
    case QAbstractSocket::ConnectionRefusedError:
        qDebug() << "Client Proxy: The connection was refused by the peer. Make sure the GraphFetcher server is running, and check that the host name and port settings are correct.";
        break;
    default:
        qDebug() << "Client Proxy: The following error occurred:" << tcpSocket->errorString();
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

