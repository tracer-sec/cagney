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

    ui->commandInput->setFocus();

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
    ui->textBuffer->scrollToBottom();
}

void BotWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    ui->textBuffer->scrollToBottom();
}

void BotWindow::commandEntered()
{
    QString line(ui->commandInput->text());
    AddMessage("> " + line);
    emit sendCommand(botId_, line);
    ui->commandInput->clear();
}
