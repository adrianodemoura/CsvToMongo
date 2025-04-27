#include "config_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Config* load_config(const char *config_file) {
    Config *config = (Config*)malloc(sizeof(Config));
    if (!config) {
        fprintf(stderr, "Erro ao alocar memória para configurações\n");
        return NULL;
    }

    // Valores padrão
    config->max_threads = 8;
    config->memory_limit_percent = 70;

    // Carrega o arquivo JSON
    struct json_object *json;
    json = json_object_from_file(config_file);
    if (!json) {
        fprintf(stderr, "Erro ao carregar arquivo de configuração: %s\n", config_file);
        free_config(config);
        return NULL;
    }

    // Extrai as configurações
    struct json_object *tmp;
    if (json_object_object_get_ex(json, "mongodb_host", &tmp))
        config->mongodb_host = strdup(json_object_get_string(tmp));
    if (json_object_object_get_ex(json, "mongodb_port", &tmp))
        config->mongodb_port = json_object_get_int(tmp);
    if (json_object_object_get_ex(json, "mongodb_database", &tmp))
        config->mongodb_database = strdup(json_object_get_string(tmp));
    if (json_object_object_get_ex(json, "mongodb_collection", &tmp))
        config->mongodb_collection = strdup(json_object_get_string(tmp));
    if (json_object_object_get_ex(json, "mongodb_username", &tmp))
        config->mongodb_username = strdup(json_object_get_string(tmp));
    if (json_object_object_get_ex(json, "mongodb_password", &tmp))
        config->mongodb_password = strdup(json_object_get_string(tmp));

    json_object_put(json);
    return config;
}

void free_config(Config *config) {
    if (!config) return;
    
    free(config->mongodb_host);
    free(config->mongodb_database);
    free(config->mongodb_collection);
    free(config->mongodb_username);
    free(config->mongodb_password);
    free(config);
} 