#ifndef PROJECT_H
#define PROJECT_H

#include <QString>

/*
 * Responsabilidades:
 * Lógica do projeto
 * Conexão com backend
 * É como se fosse um operário que vai chamar as coisas, ah o cara quer montar, chama o montador
 * O cara quer criar um diretório, cria um diretório (desnecessário uma classe pra isso, ent manda bala aqui mesmo)
 */
class Project
{
public:
    Project();
    QString resolvePath(const QString &input);
    bool createDirectory(const QString &input);
    bool deleteDirectory(const QString &input);
    QString getRootDir() const;
    bool saveFile(const QString &filepath, const QString &content);


private:
    QString rootDir = "/home/lucascacz/ProjectTestQT";
};

#endif // PROJECT_H
