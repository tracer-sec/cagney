#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QMdiSubWindow>
#include <QLabel>
#include "connection.h"
#include "botwindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void botSelected(QListWidgetItem *item);
    void dataReceived(QString line);
    void sendMessage(QString botId, QString message);
    void botWindowSelected(QMdiSubWindow *window);
    void connectionCompleted(QString hostname);
    void connectionLost();

private slots:
    void on_actionRefresh_triggered();
    void on_actionExit_triggered();
    void on_actionConnect_triggered();

    void botContextMenuRequested(const QPoint &pos);
    void botInfoRequested();

private:
    void UpdateBotList(QStringList botNames);
    void AddToBotList(QString botName);
    void RemoveFromBotList(QString botName);
    BotWindow *GetWindowByBotId(QString botId);
    Ui::MainWindow *ui;
    Connection *connection_;
    QLabel *statusLabel_;
};

#endif // MAINWINDOW_H
