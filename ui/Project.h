#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>
#include <QString>
#include "vm/vm.h"
#include "vmstate.h"

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
    bool assemble(const QStringList &input);
    bool link(const QStringList &input);
    bool build(const QStringList &input);
    bool run(const QString &binPath);
    bool load(const QString &binPath);
    bool step();
    void resetCpu();

    // bool saveFile(const QString &input);
    bool saveFileShortcut(const QString &filepath, const QString &content);

    QString getRootDir() const;

signals:
    void flagsAndReg_Modified(const VMState &state);

private:
    QString rootDir = "/home/lucascacz/ProjectTestQT";
    VM vm;
};

#endif // PROJECT_H
