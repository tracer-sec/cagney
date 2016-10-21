#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "connection.h"

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

private slots:
    void on_actionRefresh_triggered();
    void on_actionExit_triggered();

private:
    void UpdateBotList(QStringList botNames);
    Ui::MainWindow *ui;
    Connection connection_;
};

#endif // MAINWINDOW_H
