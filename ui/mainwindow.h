#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QPlainTextEdit>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;

    QFileSystemModel *modelFiles; //ponteiro pro model que vai ler os arquivos

    void openNewFile(const QString &fileName, const QString &content); //função interna pra criar as abas dinamicamente (durante exec)
    void on_treeFiles_doubleClicked(const QModelIndex &index);
    void closeTab(int index);
    void onDocumentModified(bool modified);
};
#endif // MAINWINDOW_H
