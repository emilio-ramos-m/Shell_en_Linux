#include <iostream>
#include <vector>
#include <unistd.h>

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
    return false;
}