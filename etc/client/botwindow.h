#ifndef BOTWINDOW_H
#define BOTWINDOW_H

#include <QWidget>
#include <QStringListModel>
#include <vector>

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
    void AddBotMessage(QString line);
    void AddClientMessage(QString line);
    void AddSystemMessage(QString line);
    void GotFocus();
    void Disable();

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void ScrollToEnd();

signals:
    void sendCommand(QString botId, QString message);

public slots:
    void commandEntered();

private:
    Ui::BotWindow *ui;
    QString botId_;
    std::vector<QString> commandBuffer_;
    int commandBufferIndex_;
};

#endif // BOTWINDOW_H
