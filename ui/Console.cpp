#include "Console.h"
#include "Project.h"


Console::Console(Project *project, QObject *parent)
    : QObject(parent),
    project(project) {
}

void Console::executeCommand(const QString &line) {
    QStringList tokens = line.split(" ", Qt::SkipEmptyParts);

    if(tokens.isEmpty()) { return; }

    QString command = tokens[0];

    if(command == "clear") {
        emit clearRequested(); //emite sinal
    } else if(command == "help") {
        emit output(helpText);
    } else if(command == "mkdir") {
        createDir(tokens);
    } else if(command == "rmdir") {
        deleteDir(tokens);
    } else if(command == "touch") {
        touch(tokens);
    } else if(command == "rm") {
        rm(tokens);
    } else if(command == "save") {
        save(tokens);
    } else if(command == "assemble") {
        assemble(tokens);
    } else if(command == "link") {
        //ainda n ta implementado, n tem backend
    } else if(command == "run") {
        run(tokens);
    } else if(command == "build") {
        build(tokens);
    } else if(command == "step") {
        step(tokens);
    } else if(command == "reset") {
        reset(tokens);
    } else if(command == "load") {
        load(tokens);
    } else {
        emit output("Unknown operand");
    }
}

void Console::createDir(const QStringList &tokens) {
    if(tokens.size() < 2) { //comando mkdir solto
        emit output("Missing operand on mkdir");
        return;
    }

    // espera receber mkdir "path"
    // path sempre inclui a root, mas fica abstraido pro usuario
    QString path = tokens [1];
    bool success = project->createDirectory(path);

    if(success) {
        emit output("Directory created");
    } else {
        emit output("Failed to create directory");
    }
}

void Console::deleteDir(const QStringList &tokens) {
    if(tokens.size() < 2) {
        emit output("Missing operand on rmdir");
        return;
    }

    QString path = tokens[1];
    bool success = project->deleteDirectory(path);

    if(success) {
        emit output("Directory deleted.");
    } else {
        emit output("Failed to delete directory.");
    }

}

void Console::touch(const QStringList &tokens) {
    //touch <path>
    // se path vier vazio considera a root
    if (tokens.size() < 2) {
        emit output("missing operand on touch");
        return;
    }

    QString path = tokens[1];

    bool success = project->touch(path);
    if (!success) {
        emit output("Failed to create file");
    }
}

void Console::rm(const QStringList &tokens) {
    //rm <path>
    if (tokens.size() < 2) {
        emit output("missing operand on rm");
        return;
    }

    QString path = tokens[1];
    bool success = project->remove(path);
    if(!success) {
        emit output("failed to remove file");
    }
}

void Console::save(const QStringList &tokens) {
    //save <path>
    if(tokens.size() < 2) {
        emit output("missing operand on save");
        return;
    }

    QString path = project->resolvePath(tokens[1]);
    if(path.isEmpty()) {
        emit output("invalid path");
        return;
    }
    emit saveFileRequested(path);
}

void Console::assemble(const QStringList &tokens) {
    // assemble <f1> <f2> ...
    if(tokens.size() < 2) {
        emit output("missing operand on assemble");
        return;
    }

    if(project->assemble(tokens.mid(1)).empty()) {
        emit output("Failed to assemble files");
    }
}

void Console::link(const QStringList &tokens) {
    // link <f1> <f2> ... -o <output name optional>
    // Exemplo de uso: link f1.bin f2.bin -o output.bin
    if(tokens.size() < 2) {
        emit output("missing operand on link");
        return;
    }

    QStringList inputFiles;
    QString outputName = "a.bin"; // assume a.bin

    for(int i = 1; i < tokens.size(); i++) {

        if(tokens[i] == "-o") { //se achou a flag de nome
            //verifica se tem nome
            if(i+1 < tokens.size()) {
                outputName = tokens[i+1];
                i++; // pula iteração
            } else {
                emit output("missing output filename after '-o'");
                return;
            }
        } else { //é filepath
            inputFiles.append(tokens[i]);
        }
    }

    if(inputFiles.isEmpty()) {
        emit output("no input files to link");
        return;
    }

    if(project->link(inputFiles, outputName).isEmpty()) {
        emit output("failed to link files");
    } else {
        emit output("files linked succefully to " + outputName);
    }

}

void Console::run(const QStringList &tokens) {
    //run <path>
    if(tokens.size() < 2) {
        emit output("missing operand on run");
        return;
    }

    bool success = project->run(tokens[1]);
    if(!success) {
        emit output("Failed to run .bin file");
    } else {
        emit output("Runned succesfully");
    }
}

void Console::build(const QStringList &tokens) {
    // build <f1> <f2> ... -o <output name optional>
    // Exemplo de uso: build f1.bin f2.bin -o output.bin
    if(tokens.size() < 2) {
        emit output("missing operand on build");
        return;
    }


    QStringList inputFiles;
    QString outputName = "a.bin"; // assume a.bin

    for(int i = 1; i < tokens.size(); i++) {

        if(tokens[i] == "-o") { //se achou a flag de nome
            //verifica se tem nome
            if(i+1 < tokens.size()) {
                outputName = tokens[i+1];
                i++; // pula iteração
            } else {
                emit output("missing output filename after '-o'");
                return;
            }
        } else { //é filepath
            inputFiles.append(tokens[i]);
        }
    }

    if(inputFiles.isEmpty()) {
        emit output("no input files to build");
        return;
    }

    if(project->build(inputFiles, outputName)) {
        emit output("files builded succefully to " + outputName);
    } else {
        emit output("failed to build files");
    }

}

void Console::load(const QStringList &tokens) {
    if(tokens.size() < 2) {
        emit output("missing operand on load");
        return;
    }

    if(!project->load(tokens[1])) {
        emit output("Failed to load " + tokens[1] + "into vm memory");
    } else {
        emit output(tokens[1] + " loaded to vm memory successfully");
    }
}

void Console::step(const QStringList &tokens) {
    if(!project->step()) {
        emit output("Failed to proceed with execution");
    }
}

void Console::reset(const QStringList &tokens) {
    project->resetCpu();
    emit output("CPU Reseted");
}














