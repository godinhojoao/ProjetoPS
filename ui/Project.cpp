#include "Project.h"
#include <QDir>

Project::Project() {
}

QString Project::resolvePath(const QString &input) {
    // Clean path limpa alguns errinhos da string, tipo duplicar //
    QString resolved = QDir::cleanPath(rootDir + "/" + input);

    if (!resolved.startsWith(rootDir)) {
        return QString();
    }

    // Só aceita paths que começam a partir do root.
    // simplifica a interface de terminal, já que o foco n é um terminal 1000% completo.
    return resolved;
}

QString Project::getRootDir() const {
    return rootDir;
}

bool Project::createDirectory(const QString &input) {
    QString path = resolvePath(input);

    if(path.isEmpty()) { return false; }

    return QDir().mkpath(path);
}

bool Project::deleteDirectory(const QString &input) {
    QString path = resolvePath(input);

    // Não da pra deletar algo inexistente ou a root
    if (path.isEmpty() || path == rootDir) {
        return false;
    }

    return QDir(path).removeRecursively(); //apaga absolutamente tudo dentro daquele path
}

bool Project::touch(const QString &input) {
    QString path = resolvePath(input);
    if (path.isEmpty()) { return false; }

    QFile file(path);
    return file.open(QIODevice::WriteOnly); //fecha sozinho no destrutor
}

bool Project::remove(const QString &input) {
    QString path = resolvePath(input);
    if(path.isEmpty()) { return false; }

    QFile file(path);
    return file.remove();
}

bool Project::assemble(const QStringList &input) {
    return false;
}

bool Project::link(const QStringList &input) {
    return false;
}

bool Project::build(const QStringList &input) {
    return false;
}

bool Project::run(const QString &binPath) {
    QString path = resolvePath(binPath);
    if(!vm.load(path.toStdString())) return false;

    vm.run();
    emit flagsAndReg_Modified(vm.getState());

    return true;
}

bool Project::saveFileShortcut(const QString &filepath, const QString &content) {
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