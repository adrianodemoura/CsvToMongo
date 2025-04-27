#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static FILE *log_file = NULL;

void logger_init() {
    log_file = fopen("import.log", "a");
    if (!log_file) {
        fprintf(stderr, "Erro ao abrir arquivo de log\n");
    }
}

void logger_log(LogLevel level, const char *format, ...) {
    if (!log_file) return;

    pthread_mutex_lock(&log_mutex);

    // Obtém o timestamp
    time_t now;
    time(&now);
    struct tm *tm_info = localtime(&now);
    char timestamp[26];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    // Determina o nível de log
    const char *level_str;
    switch (level) {
        case LOG_DEBUG:   level_str = "DEBUG"; break;
        case LOG_INFO:    level_str = "INFO"; break;
        case LOG_WARNING: level_str = "WARNING"; break;
        case LOG_ERROR:   level_str = "ERROR"; break;
        default:          level_str = "UNKNOWN";
    }

    // Formata a mensagem
    va_list args;
    va_start(args, format);
    char message[1024];
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    // Escreve no arquivo de log
    fprintf(log_file, "[%s] [%s] %s\n", timestamp, level_str, message);
    fflush(log_file);

    pthread_mutex_unlock(&log_mutex);
}

void logger_close() {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
} 