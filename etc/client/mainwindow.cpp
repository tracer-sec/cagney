#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "botwindow.h"
#include "connectiondialog.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    connection_(nullptr)
{
    ui->setupUi(this);

    connect(ui->botList, &QListWidget::itemDoubleClicked,
        this, &MainWindow::botSelected);
    connect(ui->messageWindowContainer, &QMdiArea::subWindowActivated,
        this, &MainWindow::botWindowSelected);

    on_actionConnect_triggered();
}

MainWindow::~MainWindow()
{
    delete connection_;
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
        auto subWindow = new QMdiSubWindow(this);
        botWindow = new BotWindow(botId);
        subWindow->setWidget(botWindow);
        s = ui->messageWindowContainer->addSubWindow(subWindow);
        connect(botWindow, &BotWindow::sendCommand,
                this, &MainWindow::sendMessage);
    }

    botWindow->show();
    ui->messageWindowContainer->setActiveSubWindow(s);
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
            botWindow->AddMessage("< " + message);
    }
}

void MainWindow::sendMessage(QString botId, QString message)
{
    if (connection_)
        connection_->Send(botId + "|" + message);
}

void MainWindow::on_actionRefresh_triggered()
{
    if (connection_)
        connection_->GetBotNames();
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionConnect_triggered()
{
    ConnectionDialog dialog(this);
    if (dialog.exec())
    {
        QString hostname = dialog.GetHostname();
        uint16_t port = dialog.GetPort();

        if (connection_)
            delete connection_;

        connection_ = new Connection(hostname, port);
        connect(connection_, &Connection::dataReceived,
            this, &MainWindow::dataReceived);
        on_actionRefresh_triggered();
    }
}

void MainWindow::botWindowSelected(QMdiSubWindow *window)
{
    for (auto childWindow : ui->messageWindowContainer->subWindowList())
    {
        if (childWindow == window)
        {
            auto w = reinterpret_cast<BotWindow *>(childWindow->widget());
            w->GotFocus();
        }
    }
}
