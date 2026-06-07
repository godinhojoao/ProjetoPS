#include "Project.h"
#include <QDir>

Project::Project() {
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

bool Project::saveFile(const QString &filepath, const QString &content) {
    QFile file(filepath);

    // Abre arquivo e valida
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    //Cria stream
    QTextStream out(&file); //abre uma stream de texto com o file
    out << content; // == getPlainText pra pegar todo conteudo mas c/ nome ruim
    file.close();

    return true;
}
