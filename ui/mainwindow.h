#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QPlainTextEdit>
#include <QShortcut>
#include "vmstate.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class Console;
class Project;
/*
 * Responsabilidades:
 * Cuidar dos objetos da própria interface
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;

    Console *console;
    Project *project;

    QFileSystemModel *modelFiles; //ponteiro pro model que vai ler os arquivos
    // QString rootDir = "/home/lucascacz/ProjectTestQT";

    void openNewFile(const QString &fileName, const QString &content, const QString &filepath); //função interna pra criar as abas dinamicamente (durante exec)
    void on_treeFiles_doubleClicked(const QModelIndex &index);
    void closeTab(int index);
    void onDocumentModified(bool modified);
    void saveCurrentFile();
    void saveFile(const QString &path);
    void onCommandEntered();
    void onRunClicked();
    void onStepClicked();
    void onResetClicked();
    void updateRegFlagTable(const VMState &state);
};
#endif // MAINWINDOW_H
