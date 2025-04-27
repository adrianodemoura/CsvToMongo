#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <sys/sysinfo.h>
#include <stdbool.h>

// Verifica se o uso de memória está dentro do limite
bool memory_check_limit(int limit_percent);

// Obtém o uso atual de memória em porcentagem
int memory_get_usage_percent();

#endif // MEMORY_MANAGER_H 