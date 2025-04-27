#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H

#include <json-c/json.h>

typedef struct {
    char *mongodb_host;
    int mongodb_port;
    char *mongodb_database;
    char *mongodb_collection;
    char *mongodb_username;
    char *mongodb_password;
    int max_threads;
    int memory_limit_percent;
} Config;

// Carrega as configurações do arquivo config.json
Config* load_config(const char *config_file);

// Libera a memória alocada para as configurações
void free_config(Config *config);

#endif // CONFIG_LOADER_H 