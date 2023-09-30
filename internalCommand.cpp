#include <iostream>
#include <vector>
#include <unistd.h>
#include <fstream>

// Función para ejecutar comandos internos
bool executeInternalCommand(const std::vector<std::string>& tokens){
    if(tokens[0] == "cd"){
        if(tokens.size() < 2){
            string homePath = getenv("HOME");
            if(chdir(homePath.c_str()) != 0){
                perror("chdir");
            }
        }else{
            if(chdir(tokens[1].c_str()) != 0){
                perror("chdir");
            }
        }
        return true;
    }
    if(tokens[0] == "history"){
        char user[LOGIN_NAME_MAX];
        getlogin_r(user, LOGIN_NAME_MAX);
        string historyFile = "/home/" + string(user) + "/.bash_history"; 
        ifstream file(historyFile);

        if (!file.is_open()) {
            cerr << "No se pudo abrir el archivo de historial de comandos." << endl;
            return 1;
        }
        string line;
        int lineNumber = 1;

        while (getline(file, line)) {
            cout << "   " << lineNumber << ":  " << line << endl;
            lineNumber++;
        }

        // Cerrar el archivo
        file.close();
        return true;
    }
    return false;
}