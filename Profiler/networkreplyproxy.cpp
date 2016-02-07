#include "networkreplyproxy.h"

int NetworkReplyProxy::startTime;
int NetworkReplyProxy::finishTime;

NetworkReplyProxy::NetworkReplyProxy(QObject *parent, QNetworkReply* reply) :
    QNetworkReply(parent), m_reply(reply)
{

    setOperation(m_reply->operation());
    setRequest(m_reply->request());
    setUrl(m_reply->url());

    //sehati
//    QFileInfo fileInfo(m_reply->url().path());
    listFile.setFileName("batchList.txt");

    listFile.open(QIODevice::Append);

    out.setDevice(&listFile);

   /* QString fileName("batch/");
    fileName.append(uniqueFileName(m_reply->url()));

    if (fileName == "batch/")
    {
        qDebug() << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%";
        fileName = "batch/index.html";
    }

    if (QFile::exists(fileName)) {
        qDebug() << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%";
        QFile::remove(fileName);
    }

    file = new QFile(fileName);
    qfi = new QFileInfo(fileName);
    if (!file->open(QIODevice::WriteOnly)) {
        qDebug() << "couldn't open the file " << fileName;
    }*/

    started = false;
    //sehati

    connect(m_reply, SIGNAL(readyRead()), this, SLOT(readInternal()));
    connect(m_reply, SIGNAL(metaDataChanged()), this, SLOT(applyMetaData()));
    //connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(errorInternal(QNetworkReply::NetworkError)));

    connect(m_reply, SIGNAL(finished()), this, SLOT(finishInternal()));
    connect(m_reply, SIGNAL(downloadProgress(qint64,qint64)), this, SIGNAL(downloadProgress(qint64,qint64)));
    connect(m_reply, SIGNAL(uploadProgress(qint64,qint64)), this, SIGNAL(uploadProgress(qint64,qint64)));

    setOpenMode(ReadOnly);
}

NetworkReplyProxy::~NetworkReplyProxy()
{
    //sehati    
//    file->close();
    //sehati

    delete m_reply;
}

void NetworkReplyProxy::setReadBufferSize(qint64 size)
{
    QNetworkReply::setReadBufferSize(size); m_reply->setReadBufferSize(size);
}

void NetworkReplyProxy::abort()
{
    m_reply->abort();
}

void NetworkReplyProxy::close()
{
    m_reply->close();
}

bool NetworkReplyProxy::isSequential() const
{
    return m_reply->isSequential();
}

qint64 NetworkReplyProxy::bytesAvailable() const
{
    return m_buffer.size() + QIODevice::bytesAvailable();
}

qint64 NetworkReplyProxy::readData(char * data, qint64 maxlen)
{
    qint64 size = qMin(maxlen, qint64(m_buffer.size()));
    memcpy(data, m_buffer.constData(), size);
    m_buffer.remove(0, size);
    return size;
}

void NetworkReplyProxy::readInternal()
{
    if (!started)
    {
        startTime = QTime::currentTime().msecsSinceStartOfDay();
        started = true;

        if(finishTime != 0){
            out << startTime - finishTime << "\t";

            out.flush();

            listFile.close();
        }
    }

    QByteArray data = m_reply->readAll();
    m_buffer += data;

    //sehati
//    file->write(data);
//    file->flush();
    //sehati

    emit readyRead();
}

void NetworkReplyProxy::finishInternal()
{
    finishTime = QTime::currentTime().msecsSinceStartOfDay();

//    out << (m_reply->url()).toEncoded().constData() << "\t" << qfi->size() << "\t" <<  QTime::currentTime().msecsSinceStartOfDay() << "\n";
//    out << (m_reply->url()).toEncoded().constData() << "\t" << startTime << "\t" <<  QTime::currentTime().msecsSinceStartOfDay() << "\n";

    emit finished();
}


void NetworkReplyProxy::applyMetaData()
{
    QList<QByteArray> headers = m_reply->rawHeaderList();

    foreach(QByteArray header, headers)
        setRawHeader(header, m_reply->rawHeader(header));

    setHeader(QNetworkRequest::ContentTypeHeader, m_reply->header(QNetworkRequest::ContentTypeHeader));
    setHeader(QNetworkRequest::ContentLengthHeader, m_reply->header(QNetworkRequest::ContentLengthHeader));
    setHeader(QNetworkRequest::LocationHeader, m_reply->header(QNetworkRequest::LocationHeader));
    setHeader(QNetworkRequest::LastModifiedHeader, m_reply->header(QNetworkRequest::LastModifiedHeader));
    setHeader(QNetworkRequest::SetCookieHeader, m_reply->header(QNetworkRequest::SetCookieHeader));

    setAttribute(QNetworkRequest::HttpStatusCodeAttribute, m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute));
    setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, m_reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute));
    setAttribute(QNetworkRequest::RedirectionTargetAttribute, m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute));
    setAttribute(QNetworkRequest::ConnectionEncryptedAttribute, m_reply->attribute(QNetworkRequest::ConnectionEncryptedAttribute));
    setAttribute(QNetworkRequest::CacheLoadControlAttribute, m_reply->attribute(QNetworkRequest::CacheLoadControlAttribute));
    setAttribute(QNetworkRequest::CacheSaveControlAttribute, m_reply->attribute(QNetworkRequest::CacheSaveControlAttribute));
    setAttribute(QNetworkRequest::SourceIsFromCacheAttribute, m_reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute));
    setAttribute(QNetworkRequest::DoNotBufferUploadDataAttribute, m_reply->attribute(QNetworkRequest::DoNotBufferUploadDataAttribute));

    emit metaDataChanged();
}

QString NetworkReplyProxy::uniqueFileName(const QUrl &url)
{
    QUrl cleanUrl = url;
    cleanUrl.setPassword(QString());
    cleanUrl.setFragment(QString());

    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(cleanUrl.toEncoded());
    // convert sha1 to base36 form and return first 8 bytes for use as string
    QByteArray id =  QByteArray::number(*(qlonglong*)hash.result().data(), 36).left(8);
    // generates <one-char subdir>/<8-char filname>
    qDebug() << "Url: " << cleanUrl.toString() << "\t" << QLatin1String(id);       

    return  QLatin1String(id);
}
