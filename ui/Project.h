#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>
#include <QString>
#include "vm/vm.h"
#include "vmstate.h"
#include "assembler/assembler.h"
#include "macro_processor/macro_processor.h"
#include "linker/linker.h"
#include <QCoreApplication>

/*
 * Responsabilidades:
 * Lógica do projeto
 * Conexão com backend
 * É como se fosse um operário que vai chamar as coisas, ah o cara quer montar, chama o montador
 * O cara quer criar um diretório, cria um diretório (desnecessário uma classe pra isso, ent manda bala aqui mesmo)
 */
class Project : public QObject {
    Q_OBJECT

public:
    Project();
    QString resolvePath(const QString &input);
    bool createDirectory(const QString &input);
    bool deleteDirectory(const QString &input);
    bool touch(const QString &input);
    bool remove(const QString &input);

    QStringList assemble(const QStringList &input);

    QString link(const QStringList &input, const QString &output = "a.bin");

    bool build(const QStringList &input, const QString &output = "a.bin");

    bool run(const QString &binPath);

    bool load(const QString &binPath);
    bool step();
    void resetCpu();

    // bool saveFile(const QString &input);
    bool saveFileShortcut(const QString &filepath, const QString &content);

    QString getRootDir() const;
    void setRootDir(const QString &path);

    bool isLoaded() const;

    VMState getVmState() const;

signals:
    void flagsAndReg_Modified(const VMState &state);

private:
    QString rootDir;

    // Project controla tudo
    VM vm;
    Assembler assembler;
    MacroProcessor mp;
    Linker linker;

};

#endif // PROJECT_H
