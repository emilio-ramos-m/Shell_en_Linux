#include <iostream>
#include <vector>
#include <sys/wait.h>

using namespace std;

void executeCommands(vector<vector<std::string>> commands){
    size_t i, n = commands.size();
    int prev_pipe, pfds[2];
        
    prev_pipe = STDIN_FILENO;

    for(i = 0; i < n - 1; i++){
        pipe(pfds);
        pid_t pid = fork();
        if(pid == 0){
            // Redireccionar pipe anterior a stdin
            if(prev_pipe != STDIN_FILENO){
                dup2(prev_pipe, STDIN_FILENO);
                close(prev_pipe);
            }

            // Redireccionar stdout to pipe actual
            dup2(pfds[1], STDOUT_FILENO);
            close(pfds[0]);
            close(pfds[1]);
                
            // Cast de string a char*
            vector<char*> cmd_args;
            for (const auto& arg : commands[i]) {
            cmd_args.push_back(const_cast<char*>(arg.c_str()));
            }
            cmd_args.push_back(nullptr);
                
            // Ejecutar comando
            execvp(cmd_args[0], cmd_args.data());
            perror("execvp failed");
            exit(1);
        }else{
            // Proceso padre
            waitpid(pid, NULL, 0);  
            close(prev_pipe);      
            prev_pipe = pfds[0];   
            close(pfds[1]);       
        }
    }
   
    // Redireccionar pipe anterior a stdin
    if (prev_pipe != STDIN_FILENO) {
        dup2(prev_pipe, STDIN_FILENO);
        close(prev_pipe);
    }
    vector<char*> last_cmd_args;
    for (const auto& arg : commands[i]) {
        last_cmd_args.push_back(const_cast<char*>(arg.c_str()));
    }
    last_cmd_args.push_back(nullptr);
    
    
    // Ejecutar comando
    execvp(last_cmd_args[0], last_cmd_args.data());        
    perror("execvp failed");
    exit(1);
}
