#include "connection.h"
#include <QThread>

Connection::Connection(QString hostname, quint16 port) :
    QObject(nullptr)
{
    socket_.connectToHost(hostname, port);
    connect(&socket_, &QTcpSocket::readyRead, this, &Connection::dataReady);
}

void Connection::GetBotNames()
{
    socket_.write("bot_list\r\n");
    socket_.waitForBytesWritten();
}

void Connection::Send(QString message)
{
    QString fullMessage = message + "\r\n";
    socket_.write(fullMessage.toStdString().c_str());
    socket_.waitForBytesWritten();
}

void Connection::dataReady()
{
    if (socket_.canReadLine())
    {
        auto data = socket_.readLine();
        QString line = QString::fromLatin1(data).trimmed();
        emit dataReceived(line);
    }
}
