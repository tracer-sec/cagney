#include "botinfodialog.h"
#include "ui_botinfodialog.h"

BotInfoDialog::BotInfoDialog(BotInfo info, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BotInfoDialog)
{
    ui->setupUi(this);
    setWindowTitle("Bot Info: " + info.botName);
    ui->nameLabel->setText(info.botName);
    ui->hostAddressLabel->setText(info.hostAddress);
    ui->timeRegisteredLabel->setText(info.timeRegistered);
}

BotInfoDialog::~BotInfoDialog()
{
    delete ui;
}
