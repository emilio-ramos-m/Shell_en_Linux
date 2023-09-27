#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

// Función para dividir una cadena en tokens
vector<string> split(const string& input, char delimiter){
    vector<string> tokens;
    string token;
    istringstream tokenStream(input);
    while (getline(tokenStream, token, delimiter)){
        tokens.push_back(token);
    }
    return tokens;
}

int main() {
    while (true) {
        cout << "mishell:$ ";
        string input;
        getline(cin, input);

        if(input.empty()) continue; // Ignorar líneas en blanco

        if(input == "exit") break; // Salir del intérprete de comandos
        
        vector<string> tokens = split(input, ' ');
        // Crear un nuevo proceso
        pid_t pid = fork();

        if(pid == -1){
            perror("fork");
            return 1;
        }

        if(pid == 0){ // Proceso hijo
            // Convertir el vector de tokens en un array de punteros de caracteres
            vector<char*> args;
            for(const string& token : tokens){
                args.push_back(const_cast<char*>(token.c_str()));
            }
            args.push_back(nullptr);

            // Ejecutar el comando
            execvp(args[0], args.data());

            // Si execvp() retorna, ha habido un error
            perror("execvp");
            return 1;
        }else{ // Proceso padre
            // Esperar a que el proceso hijo termine
            int status;
            waitpid(pid, &status, 0);

            if(WIFEXITED(status)){
                // El proceso hijo terminó normalmente
                int exit_status = WEXITSTATUS(status);
                cout << "Comando ejecutado con estado de salida: " << exit_status << endl;
            }else if(WIFSIGNALED(status)){
                // El proceso hijo fue interrumpido por una señal
                int signal_num = WTERMSIG(status);
                cerr << "Comando interrumpido por la señal: " << signal_num << endl;
            }
        }
    }

    return 0;
}
