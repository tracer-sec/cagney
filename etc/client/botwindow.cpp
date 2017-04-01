#include "botwindow.h"
#include "ui_botwindow.h"
#include <QKeyEvent>
#include <QScrollBar>

BotWindow::BotWindow(QString botId, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BotWindow),
    botId_(botId),
    commandBufferIndex_(-1)
{
    ui->setupUi(this);
    setWindowTitle(botId);

    ui->commandInput->installEventFilter(this);
    ui->commandInput->setFocus();

    connect(ui->commandInput, &QLineEdit::returnPressed,
            this, &BotWindow::commandEntered);
}

BotWindow::~BotWindow()
{
    delete ui;
}

void BotWindow::AddBotMessage(QString line)
{
    ui->textBuffer->append("< " + line);
}

void BotWindow::AddClientMessage(QString line)
{
    ui->textBuffer->setFontItalic(true);
    ui->textBuffer->append("> " + line);
    ui->textBuffer->setFontItalic(false);
}

void BotWindow::AddSystemMessage(QString line)
{
    int oldFontWeight = ui->textBuffer->fontWeight();
    ui->textBuffer->setFontWeight(oldFontWeight * 2);
    ui->textBuffer->append("* " + line);
    ui->textBuffer->setFontWeight(oldFontWeight);
}

void BotWindow::GotFocus()
{
    ui->commandInput->setFocus();
}

void BotWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    //ui->textBuffer->scrollToBottom();
    ui->textBuffer->verticalScrollBar()->setValue(ui->textBuffer->verticalScrollBar()->maximum());
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
    AddClientMessage(line);
    emit sendCommand(botId_, line);
    ui->commandInput->clear();
    commandBuffer_.push_back(line);
    commandBufferIndex_ = -1;
}
