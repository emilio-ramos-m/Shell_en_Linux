#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>

void pipeCommands(char*** commands, int num_commands) {
    int pipes[num_commands - 1][2]; // Array de tuberías
    pid_t child_pid;

    // Crear las tuberías necesarias
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }
    
    // Ejecutar comandos en secuencia
    for (int i = 0; i < num_commands; i++) {
        child_pid = fork();
        if (child_pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        
        if (child_pid == 0) { // Proceso hijo
            // Redirigir la entrada de la tubería anterior
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
                close(pipes[i - 1][1]);
            }

            // Redirigir la salida a la tubería actual (excepto para el último comando)
            if (i < num_commands - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
                close(pipes[i][0]);
            }
            
            // Ejecutar el comando actual
            execvp(commands[i][0], commands[i]);
            perror("execvp"); // Esto solo se ejecutará si hay un error en execvp
            exit(EXIT_FAILURE);
        } else { // Proceso padre
            // Cerrar los extremos no utilizados de la tubería
            if (i > 0) {
                close(pipes[i - 1][0]);
                close(pipes[i - 1][1]);
            }
            
            wait(NULL); // Esperar al proceso hijo actual
        }
    }
    
}

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

// Función para ejecutar comandos con canalización
void executeCommandWithPipe(const std::vector<std::string>& input_tokens) {
    int num_commands = 0;
    std::vector<std::vector<std::string>> commands;

    // Dividir los tokens en comandos y construir un vector de vectores de tokens
    std::vector<std::string> current_command;
    for (const std::string& token : input_tokens) {
        if (token == "|") {
            if (!current_command.empty()) {
                commands.push_back(current_command);
                current_command.clear();
            }
        } else {
            current_command.push_back(token);
        }
    }
    if (!current_command.empty()) {
        commands.push_back(current_command);
    }

    num_commands = commands.size();

    // Convertir comandos en un formato adecuado para pipeCommands
    char*** cmd_array = new char**[num_commands];
    for (int i = 0; i < num_commands; ++i) {
        std::vector<std::string>& cmd_tokens = commands[i];
        cmd_array[i] = new char*[cmd_tokens.size() + 1];
        for (size_t j = 0; j < cmd_tokens.size(); ++j) {
            cmd_array[i][j] = const_cast<char*>(cmd_tokens[j].c_str());
        }
        cmd_array[i][cmd_tokens.size()] = nullptr;
    }
     
    // Ejecutar comandos con canalización
    pipeCommands(cmd_array, num_commands);
    
    // Liberar la memoria
    for (int i = 0; i < num_commands; ++i) {
        delete[] cmd_array[i];
    }
    delete[] cmd_array;
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

        // Verificar si la entrada contiene una canalización
        bool has_pipe = false;
        for (const std::string& token : tokens) {
            if (token == "|") {
                has_pipe = true;
                break;
            }
        }

        if (has_pipe) {
            // Ejecutar comandos con canalización
           
            executeCommandWithPipe(tokens);
        } else {
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
    }

    return 0;
}