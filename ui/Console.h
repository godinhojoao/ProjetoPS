#ifndef CONSOLE_H
#define CONSOLE_H

#include <QObject>
#include <QString>
#include <QStringList>

class Project;
/*
 * Responsabilidades:
 * receber comandos
 * tratar comandos
 * chamar lógica associada
 */
class Console : public QObject {
    Q_OBJECT //qq essa linha faz?

public:
    // qq é esse parent n sei das quanta? padrao do qt mas pra q?
    explicit Console(Project *project, QObject *parent = nullptr); //arrumar construtor
    void executeCommand(const QString &line);

signals:
    void output(const QString &text);
    void clearRequested();
    void saveFileRequested(const QString &path);

private:
    QString helpText = R"(
Commands (all operations are done inside root):

  File & Directory
    mkdir <path>                  create directory (including parents)
    rmdir <path>                  remove directory and everything inside
    touch <path>                  create empty file       e.g: touch src/main.asm
    rm    <path>                  delete file             e.g: rm src/main.asm
    save  <path>                  save open file          e.g: save src/main.asm

  Build
    assemble <f1> <f2> ...                   assemble files into .o  (not implemented)
    link     <f1> <f2> ... -o <a.bin>        link .o into binary     (not implemented)
    build    <f1> <f2> ... -o <a.bin>        assemble + link         (not implemented)
    run      <path>                          load and execute binary
    load     <path>                          load a binary file to VM
    step                                     execute 1 instruction of a loaded file
    reset                                    reset cpu state

  General
    help                          show this message
    clear                         clear the terminal
)";

    void createDir(const QStringList &tokens);
    void deleteDir(const QStringList &tokens);
    void touch(const QStringList &tokens);
    void rm(const QStringList &tokens);
    void save(const QStringList &tokens);
    void assemble(const QStringList &tokens);
    void link(const QStringList &tokens);
    void run(const QStringList &tokens);
    void build(const QStringList &tokens);
    void load(const QStringList &tokens);
    void step(const QStringList &tokens);
    void reset(const QStringList &tokens);
    Project *project;


};

#endif // CONSOLE_H
