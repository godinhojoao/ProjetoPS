#ifndef CONSOLE_H
#define CONSOLE_H

#include <QObject>
#include <QString>
#include <QStringList>

class Project;

class Console : public QObject {
    Q_OBJECT //qq essa linha faz?

public:
    // qq é esse parent n sei das quanta? padrao do qt mas pra q?
    explicit Console(Project *project, QObject *parent = nullptr); //arrumar construtor
    void executeCommand(const QString &line);

signals:
    void output(const QString &text);
    void clearRequested();

private:
    void createDir(const QStringList &tokens);
    Project *project;


};

#endif // CONSOLE_H
