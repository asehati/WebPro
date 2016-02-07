#ifndef NETWORKREPLYPROXY_H
#define NETWORKREPLYPROXY_H

#include <QNetworkReply>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
 #include <QTime>
#include <QCryptographicHash>

class NetworkReplyProxy : public QNetworkReply
{
    Q_OBJECT
public:
    explicit NetworkReplyProxy(QObject *parent, QNetworkReply *reply = 0);
    ~NetworkReplyProxy();
    void abort();
    void close();
    bool isSequential() const;
    void setReadBufferSize(qint64 size);
    virtual qint64 readData(char *, qint64);
    virtual qint64 bytesAvailable()  const;    
    //void setRawHeader( const QByteArray &, const QByteArray &);

private:
    QFile listFile;
    QTextStream out;

    bool started;
    static int startTime;
    static int finishTime;

//    QFile *file;
//    QFileInfo *qfi;
//    QString fileName;
    QByteArray m_buffer;
    QNetworkReply* m_reply;
    QString uniqueFileName(const QUrl &);

signals:

public Q_SLOTS:
    void applyMetaData();
    //void errorInternal(QNetworkReply::NetworkError);
     void readInternal();
     void finishInternal();

};

#endif // NETWORKREPLYPROXY_H

