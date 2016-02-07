#include "batchsender.h"
#include <QHostAddress>
BatchSender::BatchSender(int socketDescriptor,  QObject *parent)
  :QThread(parent),socketDescriptor(socketDescriptor)
{

}

//BatchSender::BatchSender(QTcpSocket* socket, QObject *parent)
//  :QThread(parent),tcpSocket(socket)
//{

//}
//BatchSender::BatchSender(QTcpSocket* socket)
//  :tcpSocket(socket)
//{

//}

void BatchSender::run()
{
    QTcpSocket tcpSocket;
    qDebug() << "----- Thread Id in doWork(): " << thread()->currentThreadId();
//    QObject::connect(tcpSocket,SIGNAL(bytesWritten(qint64)),this,SLOT(keepTrack(qint64)));
    timer.start();
    if (!tcpSocket.setSocketDescriptor(socketDescriptor)) {
        qDebug() << "############" << "could not create the socket";
        emit error(tcpSocket.error());
        return;
    }

    qDebug() << "client address: " <<tcpSocket.peerAddress().toString();


    QFile file("batch.tar");
    file.open(QIODevice::ReadOnly);
    QByteArray tarBlock = file.readAll();

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << (quint32)tarBlock.size();
    totalBytes = (quint32)tarBlock.size() + 4;
    block.append(tarBlock);
    int createTime = timer.elapsed();
    qDebug() << "--- time to create buffer batch: " <<createTime;
//    mutex.lock();
    int numWrite = tcpSocket.write(block);
    tcpSocket.flush();
    while(tcpSocket.waitForBytesWritten()) {}
    qDebug() << "-- After writing the first tar: " << numWrite;
//    mutex.unlock();

    file.close();
    QThread::msleep(60000);
}

//void BatchSender::keepTrack(qint64 bytes)
//{
//    bytesWritten += bytes;
//    qDebug() << "just wrote " << bytes << "bytes. Total: " << bytesWritten;
//}
