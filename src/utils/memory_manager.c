#include "memory_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int memory_get_usage_percent() {
    struct sysinfo si;
    if (sysinfo(&si) != 0) {
        fprintf(stderr, "Erro ao obter informações do sistema\n");
        return -1;
    }

    // Calcula a porcentagem de memória usada
    unsigned long total_mem = si.totalram * si.mem_unit;
    unsigned long free_mem = si.freeram * si.mem_unit;
    unsigned long used_mem = total_mem - free_mem;
    
    return (int)((used_mem * 100) / total_mem);
}

bool memory_check_limit(int limit_percent) {
    int usage = memory_get_usage_percent();
    if (usage < 0) return false;
    
    return usage < limit_percent;
} 