#ifndef LOGGER_H
#define LOGGER_H

#include <pthread.h>

// Níveis de log
typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

// Inicializa o logger
void logger_init();

// Registra uma mensagem de log
void logger_log(LogLevel level, const char *format, ...);

// Fecha o logger
void logger_close();

#endif // LOGGER_H 