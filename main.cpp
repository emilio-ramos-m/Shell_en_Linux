#include <sstream>
#include <filesystem>
#include <limits.h>
#include <sys/wait.h>
#include "pipes.cpp"
#include "internalCommand.cpp"

using namespace std;

// Función para dividir una cadena en tokens
vector<vector<string>> split(const string& input, char delimiter) {
    vector<vector<string>> commands(1);
    string token;
    istringstream tokenStream(input);
    int i = 0;
    while(getline(tokenStream, token, delimiter)){
        if(token == "|"){
            commands.push_back(vector<string>());
            i++;
        } 
        else if(!token.empty()){ // Verificar que token no sea una cadena vacía
            commands[i].push_back(token);
        }
    }
    return commands;
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
    return "\033[1;32m" + string(user_aux) + "@" + string(host_aux) + "\033[0m" + ":" + 
           "\033[1;34m" + absPath + "\033[0m" +"\n $ ";
}


int main() {
    while (true) {
        cout <<getPrompt();
        string input;
        getline(cin, input);

        if (input.empty()) continue; // Ignorar líneas en blanco

        if (input == "exit") break; // Salir del intérprete de comandos

        vector<vector<string>> commands = split(input, ' ');

        // Si es un comando interno, no necesitas crear un proceso hijo
        if (executeInternalCommand(commands)) continue; 

        pid_t pid = fork();
        if (pid == 0) {
            if(executeCommands(commands) == -1) 
                cout << "Comando no encontrado: " << commands[0][0] << endl;
            return 0;
        }else{
            waitpid(pid, NULL, 0);
        }
    }
    return 0;
}
