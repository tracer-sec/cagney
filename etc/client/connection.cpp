#include "connection.h"
#include <QThread>
#include <QSslConfiguration>
#include <QSslCertificate>

Connection::Connection(QString hostname, quint16 port) :
    QObject(nullptr),
    connected_(false)
{
    connect(&socket_, &QSslSocket::encrypted, this, &Connection::connectionMade);
    connect(&socket_, &QTcpSocket::readyRead, this, &Connection::dataReady);
    connect(&socket_, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(sslErrors(const QList<QSslError> &)));

    // load and apply cert
    // TODO: get details from connection dialog
    QList<QSslCertificate> certs = QSslCertificate::fromPath("cert.pem");
    QSslConfiguration config;
    config.setCaCertificates(certs);
    socket_.setSslConfiguration(config);

    // ignore hostname errors
    QList<QSslError> expectedErrors;
    expectedErrors << QSslError(QSslError::HostNameMismatch, certs[0]);
    socket_.ignoreSslErrors(expectedErrors);

    socket_.connectToHostEncrypted(hostname, port);
}

void Connection::GetBotNames()
{
    if (connected_)
    {
        auto errorWot = socket_.write("bot_list\r\n");
        auto lol = socket_.waitForBytesWritten();
    }
}

void Connection::Send(QString message)
{
    if (connected_)
    {
        QString fullMessage = message + "\r\n";
        socket_.write(fullMessage.toStdString().c_str());
        socket_.waitForBytesWritten();
    }
}

void Connection::connectionMade()
{
    connected_ = true;
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

void Connection::sslErrors(const QList<QSslError> &errors)
{
    for (auto e : errors)
    {
        auto message = e.errorString();
        message = message;
    }
}
