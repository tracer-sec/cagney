#ifndef BOTWINDOW_H
#define BOTWINDOW_H

#include <QWidget>
#include <QStringListModel>

namespace Ui {
class BotWindow;
}

class BotWindow : public QWidget
{
    Q_OBJECT

public:
    explicit BotWindow(QString botId, QWidget *parent = 0);
    ~BotWindow();

    QString GetBotId() { return botId_; }
    void AddMessage(QString line);

signals:
    void sendCommand(QString botId, QString message);

public slots:
    void commandEntered();

private:
    Ui::BotWindow *ui;
    QString botId_;
    QStringListModel model_;
};

#endif // BOTWINDOW_H
