#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>

// Función para dividir una cadena en tokens
std::vector<std::string> split(const std::string& input, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(input);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Función para ejecutar comandos internos
bool executeInternalCommand(const std::vector<std::string>& tokens) {
    if (tokens[0] == "cd") {
        if (tokens.size() < 2) {
            std::cerr << "Uso: cd <directorio>" << std::endl;
        } else {
            if (chdir(tokens[1].c_str()) != 0) {
                perror("chdir");
            }
        }
        return true;
    }
    return false;
}

int main() {
    while (true) {
        std::cout << "mishell:$ ";
        std::string input;
        std::getline(std::cin, input);

        if (input.empty()) {
            continue; // Ignorar líneas en blanco
        }

        if (input == "exit") {
            break; // Salir del intérprete de comandos
        }

        std::vector<std::string> tokens = split(input, ' ');

        if (executeInternalCommand(tokens)) {
            continue; // Si es un comando interno, no necesitas crear un proceso hijo
        }

        // Crear un nuevo proceso
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            return 1;
        }

        if (pid == 0) { // Proceso hijo
            // Convertir el vector de tokens en un array de punteros de caracteres
            std::vector<char*> args;
            for (const std::string& token : tokens) {
                args.push_back(const_cast<char*>(token.c_str()));
            }
            args.push_back(nullptr);

            // Ejecutar el comando
            execvp(args[0], args.data());

            // Si execvp() retorna, ha habido un error
            perror("execvp");
            return 1;
        } else { // Proceso padre
            // Esperar a que el proceso hijo termine
            int status;
            waitpid(pid, &status, 0);

            if (WIFEXITED(status)) {
                // El proceso hijo terminó normalmente
                int exit_status = WEXITSTATUS(status);
                std::cout << "Comando ejecutado con estado de salida: " << exit_status << std::endl;
            } else if (WIFSIGNALED(status)) {
                // El proceso hijo fue interrumpido por una señal
                int signal_num = WTERMSIG(status);
                std::cerr << "Comando interrumpido por la señal: " << signal_num << std::endl;
            }
        }
    }

    return 0;
}
