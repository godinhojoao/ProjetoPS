#ifndef PROJECT_H
#define PROJECT_H

#include <QString>

class Project
{
public:
    Project(const QString &rootDir);
    bool createDirectory(const QString &name, const QString &path = "");
    QString getRootDir() const;

private:
    QString rootDir;

};

#endif // PROJECT_H
