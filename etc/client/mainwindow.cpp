#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "botwindow.h"

#include <QMdiSubWindow>

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pushButton, &QPushButton::clicked,
        this, &MainWindow::requestBotList);
    connect(ui->botList, &QListWidget::itemDoubleClicked,
        this, &MainWindow::botSelected);
    connect(&connection_, &Connection::dataReceived,
        this, &MainWindow::dataReceived);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::UpdateBotList(QStringList botNames)
{
    ui->botList->clear();
    ui->botList->addItems(botNames);
}

void MainWindow::botSelected(QListWidgetItem *item)
{
    QString botId = item->text();
    BotWindow *botWindow = nullptr;
    QMdiSubWindow *s = nullptr;
    for (auto childWindow : ui->messageWindowContainer->subWindowList())
    {
        auto w = reinterpret_cast<BotWindow *>(childWindow->widget());
        if (w->GetBotId() == botId)
        {
            botWindow = w;
            s = childWindow;
            break;
        }
    }

    if (botWindow == nullptr)
    {
        botWindow = new BotWindow(botId, this);
        s = ui->messageWindowContainer->addSubWindow(botWindow);
        connect(botWindow, &BotWindow::sendCommand,
                this, &MainWindow::sendMessage);
    }

    botWindow->show();
    ui->messageWindowContainer->setActiveSubWindow(s);
}

void MainWindow::requestBotList()
{
    connection_.GetBotNames();
}

void MainWindow::dataReceived(QString line)
{
    int splitIndex = line.indexOf('|');
    QString botId = line.mid(0, splitIndex);
    QString message = line.mid(splitIndex + 1);

    if (botId == "[bot_list]")
    {
        QStringList botNames = message.split(';');
        UpdateBotList(botNames);
    }
    else
    {
        BotWindow *botWindow = nullptr;
        for (auto childWindow : ui->messageWindowContainer->subWindowList())
        {
            auto w = reinterpret_cast<BotWindow *>(childWindow->widget());
            if (w->GetBotId() == botId)
            {
                botWindow = w;
                break;
            }
        }

        if (botWindow != nullptr)
            botWindow->AddMessage(message);
    }
}

void MainWindow::sendMessage(QString botId, QString message)
{
    connection_.Send(botId + "|" + message);
}
