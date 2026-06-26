#ifndef BTNRUN_POPUP_H
#define BTNRUN_POPUP_H

#include <QDialog>

namespace Ui {
class btnRun_popup;
}

class btnRun_popup : public QDialog
{
    Q_OBJECT

public:
    explicit btnRun_popup(const QString &rootPath, QWidget *parent = nullptr);
    ~btnRun_popup();

    // QString getSelectedFilePath() const;
    QString getSelectedFileName() const;

private:
    Ui::btnRun_popup *ui;
    QString rootPath;
};

#endif // BTNRUN_POPUP_H
