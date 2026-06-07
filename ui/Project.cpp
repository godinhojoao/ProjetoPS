#include "Project.h"
#include <QDir>

Project::Project(const QString &rootDir)
    : rootDir(rootDir) {
}

QString Project::getRootDir() const {
    return rootDir;
}

bool Project::createDirectory(const QString &name, const QString &path) {
    QDir dir(rootDir);

    if(path.isEmpty()) {
        return dir.mkdir(name); //retorna um booleano em caso de sucesso
    }

    return dir.mkpath(path + "/" + name);
}
