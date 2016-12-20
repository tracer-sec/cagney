#include "botwindow.h"
#include "ui_botwindow.h"
#include <QKeyEvent>

BotWindow::BotWindow(QString botId, QWidget *parent) :
    botId_(botId),
    commandBufferIndex_(-1),
    QWidget(parent),
    ui(new Ui::BotWindow)
{
    ui->setupUi(this);
    setWindowTitle(botId);

    ui->textBuffer->setModel(&model_);

    ui->commandInput->installEventFilter(this);
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

void BotWindow::GotFocus()
{
    ui->commandInput->setFocus();
}

void BotWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    ui->textBuffer->scrollToBottom();
}

bool BotWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->commandInput)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent* keyEvent = reinterpret_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Up)
            {
                 if (commandBufferIndex_ == -1)
                     commandBufferIndex_ = commandBuffer_.size() - 1;
                 else
                     commandBufferIndex_ --;
                 if (commandBufferIndex_ < 0)
                     commandBufferIndex_ = 0;
                 ui->commandInput->setText(commandBuffer_[commandBufferIndex_]);
                 return true;
            }
            else if (keyEvent->key() == Qt::Key_Down)
            {
                if (commandBufferIndex_ != -1)
                    commandBufferIndex_ ++;
                if (commandBufferIndex_ > commandBuffer_.size() - 1)
                    commandBufferIndex_ = commandBuffer_.size() - 1;
                ui->commandInput->setText(commandBuffer_[commandBufferIndex_]);
                return true;
            }
            else if (keyEvent->key() == Qt::Key_Escape)
            {
                ui->commandInput->clear();
                commandBufferIndex_ = -1;
                return true;
            }
            else
                commandBufferIndex_ = -1;
            return false;
        }
        else
            return false;
    }
    else
        return QWidget::eventFilter(watched, event);
}

void BotWindow::commandEntered()
{
    QString line(ui->commandInput->text());
    AddMessage("> " + line);
    emit sendCommand(botId_, line);
    ui->commandInput->clear();
    commandBuffer_.push_back(line);
    commandBufferIndex_ = -1;
}
