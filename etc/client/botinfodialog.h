#ifndef BOTINFODIALOG_H
#define BOTINFODIALOG_H

#include <QDialog>

namespace Ui {
class BotInfoDialog;
}

struct BotInfo
{
    QString botName;
    QString hostAddress;
    QString timeRegistered;
};

class BotInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BotInfoDialog(BotInfo info, QWidget *parent = 0);
    ~BotInfoDialog();

private:
    Ui::BotInfoDialog *ui;
};

#endif // BOTINFODIALOG_H
