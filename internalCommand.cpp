#include "daemon.cpp"

// Función para ejecutar comandos internos
bool executeInternalCommand(const std::vector<std::vector<std::string>>& tokens){
    if(tokens[0][0] == "cd"){
        if(tokens.size() < 2){
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
    }else if(tokens[0][0] == "history"){
        char user[LOGIN_NAME_MAX];
        getlogin_r(user, LOGIN_NAME_MAX);
        string historyFile = "/home/" + string(user) + "/.bash_history"; 
        ifstream file(historyFile);

        if (!file.is_open()) {
            cerr << "No se pudo abrir el archivo de historial de comandos." << endl;
            return 1;
        }

        string line;
        //Variables auxiliares para calcular el maximo de lineas
        int maxNumber = 1;
        string historyAuxFile = historyFile; 
        ifstream auxFile(historyAuxFile);
        while (getline(auxFile, line)) maxNumber++;
        //
        int maxLines;
        
        if (tokens.size()<= 1) maxLines = 30;
        else {
            for(char c : tokens[0][1]){
                if(!isdigit(c)){
                    cout << "Argumento '" << tokens[0][1] << "' invalido. Segundo argumento debe ser numero." << std::endl;
                    return false;
                }
            }
            maxLines = stoi(tokens[0][1]); //Cantidad de lineas a imprimir con el comando
        }
        int lineCount = 1, lineNumber = 1;
        while (getline(file, line)) {
            if(lineCount >= maxNumber - maxLines){
                cout << "   " << lineNumber<< ":  " << line << endl;
                lineNumber++;
            }
            lineCount++;
        }
        // Cerrar el archivo
        file.close();
        return true;
    } else if(tokens[0][0] == "daemon"){
        if (tokens.size() != 3) {
            std::cerr << "Uso: " << tokens[0][0] << " <intervalo_segundos> <tiempo_total_segundos>" << std::endl;
            exit(EXIT_FAILURE);
        }
        daemon(tokens[0]);
    }
    return false;
}