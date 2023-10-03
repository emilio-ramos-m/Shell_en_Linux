#include <iostream>
#include <vector>
#include <sys/wait.h>

using namespace std;

void pipeCommands(char*** commands, int num_commands){
    int pipes[num_commands - 1][2]; // Array de tuberías
    pid_t child_pid;

    // Crear las tuberías necesarias
    for(int i = 0; i < num_commands - 1; i++){
        if(pipe(pipes[i]) == -1){
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Ejecutar comandos en secuencia
    for(int i = 0; i < num_commands; i++){
        child_pid = fork();
        if(child_pid == -1){
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if(child_pid == 0){ // Proceso hijo
            // Redirigir la entrada de la tubería anterior
            if(i > 0){
                dup2(pipes[i - 1][0], STDIN_FILENO);
                close(pipes[i - 1][1]);
            }

            // Redirigir la salida a la tubería actual (excepto para el último comando)
            if(i < num_commands - 1){
                dup2(pipes[i][1], STDOUT_FILENO);
                close(pipes[i][0]);
            }
            
            // Ejecutar el comando actual
            execvp(commands[i][0], commands[i]);
            perror("execvp"); // Esto solo se ejecutará si hay un error en execvp
            exit(EXIT_FAILURE);
        }else{ // Proceso padre
            // Cerrar los extremos no utilizados de la tubería
            if(i > 0){
                close(pipes[i - 1][0]);
                close(pipes[i - 1][1]);
            }
            wait(NULL); // Esperar al proceso hijo actual
        }
    }
    
}

void executeCommandWithPipe(const vector<string>& input_tokens) {
    int num_commands = 0;
    vector<vector<string>> commands;

    // Dividir los tokens en comandos y construir un vector de vectores de tokens
    vector<string> current_command;
    for(const string& token : input_tokens){
        if(token == "|"){
            if(!current_command.empty()){
                commands.push_back(current_command);
                current_command.clear();
            }
        }else{
            current_command.push_back(token);
        }
    }
    if(!current_command.empty()){
        commands.push_back(current_command);
    }

    num_commands = commands.size();
    // Convertir comandos en un formato adecuado para pipeCommands
    char*** cmd_array = new char**[num_commands];
    for(int i = 0; i < num_commands; ++i){
        vector<string>& cmd_tokens = commands[i];
        cmd_array[i] = new char*[cmd_tokens.size() + 1];
        for(size_t j = 0; j < cmd_tokens.size(); ++j){
            cmd_array[i][j] = const_cast<char*>(cmd_tokens[j].c_str());
        }
        cmd_array[i][cmd_tokens.size()] = nullptr;
    }
    // Ejecutar comandos con canalización
    pipeCommands(cmd_array, num_commands);
    // Liberar la memoria
    for(int i = 0; i < num_commands; ++i){
        delete[] cmd_array[i];
    }
    delete[] cmd_array;
}