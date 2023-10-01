#include <iostream>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <cstdlib>

using namespace std;

void daemonize() {
    // Crear un proceso hijo
    pid_t pid = fork();

    // Terminar el proceso padre
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // Cambiar el umask
    umask(0);

    // Crear una nueva sesión y convertirse en líder de sesión
    if (setsid() == -1) {
        exit(EXIT_FAILURE);
    }

    // Cerrar los descriptores de archivo estándar
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

void logSystemInfo() {
    // Abrir el archivo /proc/cpuinfo
    ifstream cpuinfo("/proc/cpuinfo");
    if (!cpuinfo) {
        syslog(LOG_ERR, "Error abriendo /proc/cpuinfo");
        exit(EXIT_FAILURE);
    }

    // Leer la información del archivo
    string line;
    while (getline(cpuinfo, line)) {
        // Buscar las líneas que contienen la información requerida
        if (line.find("processes") != string::npos ||
            line.find("procs_running") != string::npos ||
            line.find("procs_blocked") != string::npos) {
            // Escribir la línea en el log del sistema
            syslog(LOG_INFO, "%s", line.c_str());
        }
    }

    // Cerrar el archivo /proc/cpuinfo
    cpuinfo.close();
}

void daemon(const vector<string>& tokens) {
    // Verificar el número de argumentos
    if (tokens.size() != 3) {
        cerr << "Uso: " << tokens[0] << " <intervalo_segundos> <tiempo_total_segundos>" << endl;
        exit(EXIT_FAILURE);
    }

    // Obtener los argumentos de la línea de comandos
    int intervalo = stoi(tokens[1]);
    int tiempoTotal = stoi(tokens[2]);

    // Demonizar el proceso
    daemonize();

    // Inicializar el sistema de registro
    openlog("matt_daemon", LOG_PID, LOG_USER);

    // Loop para medir y registrar la información
    for (int tiempoTranscurrido = 0; tiempoTranscurrido < tiempoTotal; tiempoTranscurrido += intervalo) {
        logSystemInfo();
        sleep(intervalo);
    }
    // Cerrar el sistema de registro
    closelog();
}

