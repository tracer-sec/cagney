#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>

namespace Ui {
class ConnectionDialog;
}

class ConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionDialog(QWidget *parent = 0);
    ~ConnectionDialog();

    QString GetHostname();
    quint16 GetPort();

    void accept() override;

private:
    Ui::ConnectionDialog *ui;
};

#endif // CONNECTIONDIALOG_H
