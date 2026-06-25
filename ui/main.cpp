#include "mainwindow.h"
#include "Project.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Project *project = new Project();
    project->setRootDir(QApplication::applicationDirPath() + "/../../../workspace");

    MainWindow w(project);
    w.setWindowIcon(QIcon(":/z80icon.png"));
    w.show();

    return QApplication::exec();
}
