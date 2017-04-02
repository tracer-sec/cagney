#include "connectiondialog.h"
#include "ui_connectiondialog.h"
#include <QMessageBox>
#include <QFileDialog>

ConnectionDialog::ConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionDialog)
{
    ui->setupUi(this);

    connect(ui->browseCertButton, &QPushButton::clicked,
            this, &ConnectionDialog::browseCertButtonClicked);
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

QString ConnectionDialog::GetPassword()
{
    return ui->passwordEdit->text();
}

QString ConnectionDialog::GetCertPath()
{
    return ui->certPathEdit->text();
}

void ConnectionDialog::accept()
{
    bool validNumber;
    ui->portEdit->text().toUShort(&validNumber);
    if (ui->hostEdit->text().length() > 0
            && validNumber
            && ui->certPathEdit->text().length() > 0)
        QDialog::accept();
    else
    {
        QMessageBox msg(this);
        msg.setWindowTitle("Connection Details");
        msg.setText("Please ensure that the connection details are valid");
        msg.exec();
    }
}

void ConnectionDialog::browseCertButtonClicked()
{
    auto filename = QFileDialog::getOpenFileName(this, "Select certificate file", "", "Certificate files (*.cer *.pem);; All files (* *.*)");
    if (!filename.isNull())
        ui->certPathEdit->setText(filename);
}
