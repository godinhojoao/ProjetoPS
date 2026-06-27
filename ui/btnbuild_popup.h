#ifndef BTNBUILD_POPUP_H
#define BTNBUILD_POPUP_H

#include <QDialog>
#include "Project.h"

namespace Ui {
class btnBuild_popup;
}

class btnBuild_popup : public QDialog
{
    Q_OBJECT

public:
    explicit btnBuild_popup(Project *project, const QString &action, QWidget *parent = nullptr);
    ~btnBuild_popup();

    QStringList getFilesSelected() const;

private slots:
    void on_buttonBox_accepted();

private:
    Ui::btnBuild_popup *ui;

    void onAddFile();
    void onRemoveFile();
    // void onOkClicked();

    QString rootPath;
    QString action;

    Project *project;
};

#endif // BTNBUILD_POPUP_H
