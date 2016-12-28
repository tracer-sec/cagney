#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSslSocket>

class Connection : public QObject
{
    Q_OBJECT

public:
    Connection(QObject *parent, QString hostname, quint16 port, QString certPath);

    void Connect();
    void GetBotNames();
    void Send(QString message);

public slots:
    void connectionMade();
    void dataReady();
    void sslErrors(const QList<QSslError> &errors);

signals:
    void dataReceived(QString line);
    void connectionCompleted();

private:
    QSslSocket socket_;
    QString hostname_;
    quint16 port_;
    bool connected_;
};

#endif // CONNECTION_H
