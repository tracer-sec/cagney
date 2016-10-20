#ifndef CONNECTION_H
#define CONNECTION_H

#include <QTcpSocket>

class Connection : public QObject
{
    Q_OBJECT

public:
    Connection();

    void GetBotNames();
    void Send(QString message);

public slots:
    void dataReady();

signals:
    void dataReceived(QString line);

private:
    QTcpSocket socket_;
};

#endif // CONNECTION_H
