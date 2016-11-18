#include "connectiondialog.h"
#include "ui_connectiondialog.h"
#include <QMessageBox>

ConnectionDialog::ConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionDialog)
{
    ui->setupUi(this);
}

ConnectionDialog::~ConnectionDialog()
{
    delete ui;
}

QString ConnectionDialog::GetHostname()
{
    return ui->hostEdit->text();
}

quint16 ConnectionDialog::GetPort()
{
    return ui->portEdit->text().toUShort();
}

void ConnectionDialog::accept()
{
    bool validNumber;
    ui->portEdit->text().toUShort(&validNumber);
    if (ui->hostEdit->text().length() > 0 && validNumber)
        QDialog::accept();
    else
    {
        QMessageBox msg(this);
        msg.setWindowTitle("Connection Details");
        msg.setText("Please ensure that the connection details are valid");
        msg.exec();
    }
}
