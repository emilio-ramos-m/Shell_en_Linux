#include "daemon.cpp"

// Función para ejecutar comandos internos
bool executeInternalCommand(const std::vector<std::vector<std::string>>& tokens){
    if(tokens[0][0] == "cd"){
        if(tokens[0].size() < 2){
            string homePath = getenv("HOME");
            if(chdir(homePath.c_str()) != 0){
                perror("chdir");
            }
        }else{
            if(chdir(tokens[0][1].c_str()) != 0){
                perror("chdir");
            }
        }
        return true;
    }
    
    else if(tokens[0][0] == "history"){
        if(tokens[0].size() >= 3 ){
            cout << "Entrada debe ser de la forma history o history <-número_de_líneas>" << endl;
            return true;
        }else{
            char user[LOGIN_NAME_MAX];
            getlogin_r(user, LOGIN_NAME_MAX);
            string historyFile = "/home/" + string(user) + "/.bash_history"; 
            ifstream file(historyFile);
            if (!file.is_open()) {
                cout << "No se pudo abrir el archivo de historial de comandos." << endl;
                return true;
            }

            string line;
            int lineNumber = 1;
            if(tokens[0].size() == 1){
                while (getline(file, line)){
                    cout << "  " << lineNumber<< ":  " << line << endl;
                    lineNumber++; 
                }
            }
            if(tokens[0].size() == 2){
                string arg2 = tokens[0][1];
                // valida que segundo argumento sea de la forma <-numero>
                if(arg2[0] != '-') {
                    cout << "Entrada debe ser de la forma history <-número_de_líneas>" << endl;
                    file.close();
                    return true;
                }else{
                    arg2.erase(0,1); //Tiene - , lo elimina para verificar la siguiente parte
                    for(char c : arg2){
                        if(!isdigit(c)){
                            cout << "Argumento '" << arg2 << "' invalido. Segundo argumento debe ser numero." << endl;
                            file.close();
                            return true;
                        }
                    }
                    //Variables auxiliares
                    int maxNumber = 1;
                    string historyAuxFile = historyFile; 
                    ifstream auxFile(historyAuxFile);
                    while (getline(auxFile, line)) maxNumber++;

                    int maxLines = stoi(arg2), lineCount = 1;
                    while (getline(file, line)) {
                        if(lineCount >= maxNumber - maxLines){
                            cout << "  " << lineCount << ":  " << line << endl;
                            lineNumber++;
                        }
                    lineCount++;
                    }
                }
            }
            file.close();
        }
        return true;
    } 
    
    else if(tokens[0][0] == "daemon"){
        if (tokens[0].size() != 3) {
            cout << "Uso: " << tokens[0][0] << " <intervalo_segundos> <tiempo_total_segundos>" << endl;
            return true;
        }
        daemon(tokens[0]);
    }

    return false;
}