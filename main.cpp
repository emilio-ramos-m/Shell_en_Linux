#include <sstream>
#include <filesystem>
#include <limits.h>
#include "pipes.cpp"
#include "internalCommand.cpp"

using namespace std;

// Función para dividir una cadena en tokens
vector<string> split(const string& input, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(input);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

string getPrompt(){
    char host_aux[HOST_NAME_MAX];
    char user_aux[LOGIN_NAME_MAX];
    gethostname(host_aux, HOST_NAME_MAX);
    getlogin_r(user_aux, LOGIN_NAME_MAX);
    string absPath = filesystem::current_path();
    string homePath = getenv("HOME");
    if(absPath.find(homePath) != string::npos){
        absPath.replace(absPath.find(homePath), homePath.length(), "~");
    }
    return "\033[1;32m" + string(user_aux) + "@" + string(host_aux) + "\033[0m" + ":" + "\033[1;34m" +absPath + "\033[0m" +"\n $ ";
}


int main() {
    while (true) {
        cout <<getPrompt();
        string input;
        getline(cin, input);

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
        for(const string& token : tokens){
            if(token == "|"){
                has_pipe = true;
                break;
            }
        }

        if(has_pipe) executeCommandWithPipe(tokens);    // Ejecutar comandos con canalización
        else{
            // Crear un nuevo proceso
            pid_t pid = fork();
            if(pid == -1){
                perror("fork");
                return 1;
            }

            if(pid == 0) { // Proceso hijo
                // Convertir el vector de tokens en un array de punteros de caracteres
                vector<char*> args;
                for (const string& token : tokens) {
                    args.push_back(const_cast<char*>(token.c_str()));
                }
                args.push_back(nullptr);

                // Ejecutar el comando
                execvp(args[0], args.data());
                perror("execvp");
                return 1;
            }else{ // Proceso padre
                // Esperar a que el proceso hijo termine
                int status;
                waitpid(pid, &status, 0);
                if(WIFSIGNALED(status)){
                    // El proceso hijo fue interrumpido por una señal
                    int signal_num = WTERMSIG(status);
                    cerr << "Comando interrumpido por la señal: " << signal_num << endl;
                }
            }
        }
    }

    return 0;
}
