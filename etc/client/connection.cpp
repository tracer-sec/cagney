#include "connection.h"
#include <QThread>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QMessageBox>

Connection::Connection(QObject *parent, QString hostname, quint16 port, QString password, QString certPath) :
    QObject(parent),
    hostname_(hostname),
    port_(port),
    password_(password),
    connected_(false)
{
    connect(&socket_, &QSslSocket::encrypted, this, &Connection::connectionMade);
    connect(&socket_, &QTcpSocket::readyRead, this, &Connection::dataReady);
    connect(&socket_, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(sslErrors(const QList<QSslError> &)));

    // load and apply cert
    QList<QSslCertificate> certs = QSslCertificate::fromPath(certPath);
    QSslConfiguration config;
    config.setCaCertificates(certs);
    config.setProtocol(QSsl::TlsV1_2);
    socket_.setSslConfiguration(config);

    // ignore hostname errors
    if (certs.length() > 0)
    {
        QList<QSslError> expectedErrors;
        expectedErrors << QSslError(QSslError::HostNameMismatch, certs[0]);
        socket_.ignoreSslErrors(expectedErrors);
    }
}

void Connection::Connect()
{
    socket_.connectToHostEncrypted(hostname_, port_);
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
    QString fullMessage = password_ + "\r\n";
    socket_.write(fullMessage.toStdString().c_str());
    socket_.waitForBytesWritten();
}

void Connection::dataReady()
{
    if (socket_.canReadLine())
    {
        auto data = socket_.readLine();
        QString line = QString::fromLatin1(data).trimmed();
        if (connected_)
        {
            emit dataReceived(line);
        }
        else
        {
            if (line == "OK")
            {
                connected_ = true;
                emit connectionCompleted();
            }
            else if (line == "AUTH_FAILED")
            {
                QMessageBox::warning(nullptr, "Connection Errors", "Authentication failed");
            }
        }
    }
}

void Connection::sslErrors(const QList<QSslError> &errors)
{
    QString allErrors;
    for (auto e : errors)
    {
        allErrors += e.errorString();
        if (e.error() == QSslError::HostNameMismatch)
            allErrors += " (IGNORED)";
        allErrors += "\n";
    }

    if (allErrors.length() > 0)
    {
        QMessageBox::warning(nullptr, "SSL Connection Errors", allErrors);
    }
}
