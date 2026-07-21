#include "Project.h"
#include <QDir>
#include <QDebug>

Project::Project() {
}

QString Project::resolvePath(const QString &input) {
    // Clean path limpa alguns errinhos da string, tipo duplicar //
    QString cleanRoot = QDir::cleanPath(rootDir);
    QString resolved = QDir::cleanPath(cleanRoot + "/" + input);

    if (!resolved.startsWith(cleanRoot)) {
        return QString();
    }

    // Só aceita paths que começam a partir do root.
    // simplifica a interface de terminal, já que o foco n é um terminal 1000% completo.
    return resolved;
}

QString Project::getRootDir() const {
    return rootDir;
}

void Project::setRootDir(const QString &path) {
    rootDir = path;
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

QStringList Project::assemble(const QStringList &input) {
    QStringList binPaths;
    //assemble <f1> <f2> ...
    for (const QString &file : input) {
        QString realFilePath = resolvePath(file);
        //chama processador de macro
        mp.reset(); //garante q ta limpo pra processar novos macros
        mp.findAndStoreMacros(realFilePath.toStdString());

        QString expandedFilePath = realFilePath + "-expanded.asm";
        QFile tmpFile(expandedFilePath);

        if(!tmpFile.open(QIODevice::WriteOnly | QIODevice::Text)) { return {}; }
        QTextStream out(&tmpFile);
        out << QString::fromStdString(mp.getExpandedCode());
        tmpFile.close();

        //envia arq expandido temporario pro assembler
        QString binPath = QString::fromStdString(assembler.assemble(expandedFilePath.toStdString()));
        binPaths.append(binPath);

        //dps de gerar .bin, exclui o arquivo temporário
        // o normal seria isso, mas ele diz na especificação q quer, ent n deleta
        // QFile::remove(expandedFilePath);
    }
    return binPaths;
}

QString Project::link(const QStringList &input, const QString &output) {
    //recebe a lista de .bin e devolve o Path de um .bin final
    // output é nome do arquivo final
    // linker.link()
    std::vector<std::string> linkerInput;
    for(const QString& filepath : input) {
        QString resolved = resolvePath(filepath);
        linkerInput.push_back(resolved.toStdString());
    }

    QString outputResolved = resolvePath(output);
    if (linker.link(linkerInput, outputResolved.toStdString())) {
        return outputResolved;
    }

    return "";
}

bool Project::build(const QStringList &input, const QString &output) {
    QStringList filesAssembled = assemble(input);
    if(filesAssembled.empty()) return false;


    QString finalBinPath = link(input, output);
    if(finalBinPath.isEmpty()) return false;

    //acho q esse run n é necessário aqui, mas caso for...
    // if(!run(finalBinPath)) return false;

    return true; //gerou o binario
}

bool Project::run(const QString &binPath) {
    QString path = resolvePath(binPath);

    // executa a função de load respectiva
    if(binPath.endsWith(".bin")) {
        if(!vm.load(path.toStdString())) return false;

    } else if(binPath.endsWith(".o")) {
        if(!vm.loadObject(path.toStdString())) return false;
    }

    vm.run();
    emit flagsAndReg_Modified(vm.getState());

    return true;
}

bool Project::load(const QString &binPath) {
    QString path = resolvePath(binPath) ;
    vm.reset();

    // executa a função de load respectiva
    if(binPath.endsWith(".bin")) {
        if(!vm.load(path.toStdString())) return false;

    } else if(binPath.endsWith(".o")) {
        if(!vm.loadObject(path.toStdString())) return false;
    }

    emit flagsAndReg_Modified(vm.getState());
    return true;
}

bool Project::step() {
    emit flagsAndReg_Modified(vm.getState());
    return vm.step();
}

void Project::resetCpu() {
    vm.reset();
    emit flagsAndReg_Modified(vm.getState());
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

bool Project::isLoaded() const{
    return vm.isLoaded();
}

VMState Project::getVmState() const {
    return vm.getState();
}

const Memory& Project::getMemory() const {
    return vm.getMemory();
}
