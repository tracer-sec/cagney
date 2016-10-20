#include "botwindow.h"
#include "ui_botwindow.h"

BotWindow::BotWindow(QString botId, QWidget *parent) :
    botId_(botId),
    QWidget(parent),
    ui(new Ui::BotWindow)
{
    ui->setupUi(this);
    setWindowTitle(botId);

    ui->textBuffer->setModel(&model_);

    connect(ui->commandInput, &QLineEdit::returnPressed,
            this, &BotWindow::commandEntered);
}

BotWindow::~BotWindow()
{
    delete ui;
}

void BotWindow::AddMessage(QString line)
{
    model_.insertRow(model_.rowCount());
    QModelIndex index = model_.index(model_.rowCount() - 1);
    model_.setData(index, line);
}

void BotWindow::commandEntered()
{
    emit sendCommand(botId_, ui->commandInput->text());
    ui->commandInput->clear();
}
