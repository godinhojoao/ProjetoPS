#ifndef BTNBUILD_POPUP_H
#define BTNBUILD_POPUP_H

#include <QDialog>

namespace Ui {
class btnBuild_popup;
}

class btnBuild_popup : public QDialog
{
    Q_OBJECT

public:
    explicit btnBuild_popup(const QString &rootPath, QWidget *parent = nullptr);
    ~btnBuild_popup();

private:
    Ui::btnBuild_popup *ui;

    void onAddFile();
    void onRemoveFile();

    QString rootPath;
};

#endif // BTNBUILD_POPUP_H
