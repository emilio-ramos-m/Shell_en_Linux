#include <unistd.h>
#include <vector>
#include <cstring>

using namespace std;

int SpawnProcess(int in, int out, const vector<string> &args){
    pid_t pid = fork();
    if(pid == 0){
        if(in != 0){
            dup2(in, 0);
            close(in);
        }
        if(out != 1){
            dup2(out, 1);
            close(out);
        }

        vector<char*> argv;
        for(const auto &arg : args){
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);

        return execvp(argv[0], argv.data());
    }

  return pid;
}

int executeCommands(const vector<vector<string>> &commands){
    int in = 0;
    int fd[2];

    for(size_t i = 0; i < commands.size() - 1; ++i){
        pipe(fd);

        SpawnProcess(in, fd[1], commands[i]);

        close(fd[1]);
        in = fd[0];
    }

    if(in != 0) dup2(in, 0);

    vector<char*> argv;
    for(const auto &arg : commands.back()){
        argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);

    return execvp(argv[0], argv.data());
}

