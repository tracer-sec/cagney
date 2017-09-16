#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connectiondialog.h"
#include "botinfodialog.h"
#include <QJsonDocument>
#include <QJsonObject>

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    connection_(nullptr)
{
    ui->setupUi(this);

    statusLabel_ = new QLabel(this);
    statusLabel_->setText("DISCONNECTED");
    ui->statusBar->addPermanentWidget(statusLabel_);

    connect(ui->botList, &QListWidget::itemDoubleClicked,
        this, &MainWindow::botSelected);
    connect(ui->botList, &QListWidget::customContextMenuRequested,
        this, &MainWindow::botContextMenuRequested);
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

void MainWindow::AddToBotList(QString botName)
{
    ui->botList->addItem(botName);
}

void MainWindow::RemoveFromBotList(QString botName)
{
    // Really, Qt?
    auto items = ui->botList->findItems(botName, Qt::MatchFixedString);
    auto index = ui->botList->row(items[0]);
    auto item = ui->botList->takeItem(index);
    delete item;
}

BotWindow *MainWindow::GetWindowByBotId(QString botId)
{
    for (auto childWindow : ui->messageWindowContainer->subWindowList())
    {
        auto w = reinterpret_cast<BotWindow *>(childWindow->widget());
        if (w->GetBotId() == botId)
        {
            return w;
        }
    }

    return nullptr;
}

void MainWindow::botSelected(QListWidgetItem *item)
{
    QString botId = item->text();
    BotWindow *botWindow = GetWindowByBotId(botId);

    if (botWindow == nullptr)
    {
        auto subWindow = new QMdiSubWindow(this);
        botWindow = new BotWindow(botId);
        subWindow->setWidget(botWindow);
        ui->messageWindowContainer->addSubWindow(subWindow);
        connect(botWindow, &BotWindow::sendCommand,
                this, &MainWindow::sendMessage);
    }

    botWindow->show();

    QMdiSubWindow *s = reinterpret_cast<QMdiSubWindow *>(botWindow->parent());
    ui->messageWindowContainer->setActiveSubWindow(s);
}

void MainWindow::botContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = ui->botList->mapToGlobal(pos);

    QMenu menu;
    menu.addAction("Properties", this, &MainWindow::botInfoRequested);
    menu.exec(globalPos);
}

void MainWindow::botInfoRequested()
{
    QListWidgetItem *item = ui->botList->selectedItems()[0];
    QString botId = item->text();
    connection_->Send("info_" + botId);
}

void MainWindow::dataReceived(QString line)
{
    int splitIndex = line.indexOf('|');
    QString botId = line.mid(0, splitIndex);
    QString message = line.mid(splitIndex + 1);

    if (botId == "[bot_list]")
    {
        if (message.length() > 0)
        {
            QStringList botNames = message.split(';');
            UpdateBotList(botNames);
        }
    }
    else if (botId == "[bot_joined]")
    {
        AddToBotList(message);
    }
    else if (botId == "[bot_left]")
    {
        BotWindow *botWindow = GetWindowByBotId(message);
        if (botWindow != nullptr)
        {
            botWindow->AddSystemMessage("Bot left");
            botWindow->Disable();
        }
        RemoveFromBotList(message);
    }
    else if (botId.startsWith("info_"))
    {
        botId = botId.mid(5);
        QJsonDocument data = QJsonDocument::fromJson(message.toUtf8());
        QJsonObject root = data.object();

        BotInfo botInfo;
        botInfo.botName = botId;
        botInfo.hostAddress = root["addr"].toString();
        botInfo.timeRegistered = root["reg"].toString();
        BotInfoDialog dialog(botInfo, this);
        dialog.exec();
    }
    else
    {
        BotWindow *botWindow = GetWindowByBotId(botId);
        if (botWindow != nullptr)
            botWindow->AddBotMessage(message);
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
        QString password = dialog.GetPassword();
        QString certPath = dialog.GetCertPath();

        if (connection_)
            delete connection_;

        setWindowTitle("Cagney");
        statusLabel_->setText("DISCONNECTED");

        connection_ = new Connection(this, hostname, port, password, certPath);
        connect(connection_, &Connection::connectionCompleted,
            this, &MainWindow::connectionCompleted);
        connect(connection_, &Connection::dataReceived,
            this, &MainWindow::dataReceived);
        connection_->Connect();
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

void MainWindow::connectionCompleted(QString hostname)
{
    setWindowTitle("Cagney - " + hostname);
    statusLabel_->setText("CONNECTED");
    on_actionRefresh_triggered();
}
