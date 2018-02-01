#ifndef BATCHSENDER_H
#define BATCHSENDER_H
#include <QThread>
#include <QTcpSocket>
#include <QFile>
#include <QMutex>
#include <QTime>

class BatchSender:  public QThread//public QObject
{
    Q_OBJECT

   public:
       BatchSender(int socketDescriptor, QObject *parent);
//      BatchSender(QTcpSocket* socket, QObject *parent);
//       BatchSender(QTcpSocket* socket);
//       void doWork();
        void run();

   signals:
       void error(QTcpSocket::SocketError socketError);

   private:
       int socketDescriptor;
       QTcpSocket* tcpSocket;
       int totalBytes;
       int bytesWritten;
       QMutex mutex;
       QTime timer;
//   private slots:
//         void keepTrack(qint64 bytes);
};

#endif // BATCHSENDER_H
