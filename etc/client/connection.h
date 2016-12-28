#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSslSocket>

class Connection : public QObject
{
    Q_OBJECT

public:
    Connection(QString hostname, quint16 port, QString certPath);

    void GetBotNames();
    void Send(QString message);

public slots:
    void connectionMade();
    void dataReady();
    void sslErrors(const QList<QSslError> &errors);

signals:
    void dataReceived(QString line);

private:
    QSslSocket socket_;
    bool connected_;
};

#endif // CONNECTION_H
