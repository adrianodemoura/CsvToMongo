#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <unistd.h>
#include <json-c/json.h>
#include <mongoc/mongoc.h>
#include <time.h>

#include "config/config_loader.h"
#include "mongodb/mongodb_client.h"
#include "utils/memory_manager.h"
#include "utils/logger.h"
#include "utils/string_utils.h"

#define MAX_THREADS 16
#define LOG_WARN 2  // Adicionando definição do LOG_WARN

// Variáveis globais para contagem
static int total_lines_read = 0;
static int total_documents_inserted = 0;
static pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    char *filename;
    Config *config;
} ThreadData;

// Função para verificar se um arquivo segue o padrão pagina_nnnn.csv
bool is_valid_filename(const char* filename) {
    if (!filename) return false;
    
    // Verifica se começa com "pagina_"
    if (strncmp(filename, "pagina_", 7) != 0) return false;
    
    // Verifica se tem 4 dígitos
    for (int i = 7; i < 11; i++) {
        if (!isdigit(filename[i])) return false;
    }
    
    // Verifica se termina com .csv
    if (strcmp(filename + 11, ".csv") != 0) return false;
    
    return true;
}

// Função para comparar nomes de arquivos
int compare_files(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

// Função para remover aspas de uma string
char* remove_quotes(const char* str) {
    if (!str) return NULL;
    
    size_t len = strlen(str);
    if (len < 2) return strdup(str);
    
    // Verifica se começa e termina com aspas
    if (str[0] == '"' && str[len-1] == '"') {
        char* result = malloc(len-1);
        if (!result) return NULL;
        
        strncpy(result, str+1, len-2);
        result[len-2] = '\0';
        return result;
    }
    
    return strdup(str);
}

// Função para ler os campos do arquivo fields.txt
char** read_fields_from_file(const char* filename, int* field_count) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Erro ao abrir arquivo fields.txt\n");
        return NULL;
    }

    char** fields = NULL;
    char line[256];
    int count = 0;

    while (fgets(line, sizeof(line), file)) {
        // Remove quebra de linha
        line[strcspn(line, "\n")] = 0;
        
        // Remove espaços em branco
        char* trimmed = line;
        while (*trimmed == ' ') trimmed++;
        
        if (strlen(trimmed) > 0) {
            fields = realloc(fields, (count + 1) * sizeof(char*));
            fields[count] = strdup(trimmed);
            count++;
        }
    }

    fclose(file);
    *field_count = count;
    return fields;
}

// Função para encontrar o índice de um campo no cabeçalho
int find_field_index(char** header, int header_count, const char* field_name) {
    for (int i = 0; i < header_count; i++) {
        if (header[i] && strcmp(header[i], field_name) == 0) {
            return i;
        }
    }
    return -1;
}

// Função para carregar o mapeamento de campos
struct json_object* load_field_mapping() {
    struct json_object* mapping = json_object_from_file("config/field_mapping.json");
    if (!mapping) {
        logger_log(LOG_ERROR, "Erro ao carregar mapeamento de campos");
        return NULL;
    }
    return mapping;
}

void *process_file(void *arg) {
    ThreadData *data = (ThreadData*)arg;
    if (!data || !data->filename || !data->config) {
        logger_log(LOG_ERROR, "Dados da thread inválidos");
        return NULL;
    }

    // Carrega o mapeamento de campos
    struct json_object* mapping = load_field_mapping();
    if (!mapping) {
        return NULL;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "files_csv/%s", data->filename);

    // Inicializa o cliente MongoDB
    char uri[256];
    snprintf(uri, sizeof(uri), "mongodb://%s:%s@%s:%d",
        data->config->mongodb_username,
        data->config->mongodb_password,
        data->config->mongodb_host,
        data->config->mongodb_port);

    MongoDBClient *client = mongodb_client_init(uri, 
        data->config->mongodb_database,
        data->config->mongodb_collection);

    if (!client) {
        logger_log(LOG_ERROR, "Erro ao inicializar cliente MongoDB para arquivo: %s", filepath);
        json_object_put(mapping);
        return NULL;
    }

    // Abre o arquivo CSV
    FILE *file = fopen(filepath, "r");
    if (!file) {
        logger_log(LOG_ERROR, "Erro ao abrir arquivo: %s", filepath);
        mongodb_client_close(client);
        json_object_put(mapping);
        return NULL;
    }

    // Lê o cabeçalho (primeira linha)
    char *line = NULL;
    size_t line_size = 0;
    if (getline(&line, &line_size, file) == -1) {
        logger_log(LOG_ERROR, "Arquivo vazio: %s", filepath);
        fclose(file);
        mongodb_client_close(client);
        json_object_put(mapping);
        return NULL;
    }

    // Verifica se há mais linhas além do cabeçalho
    if (getline(&line, &line_size, file) == -1) {
        logger_log(LOG_ERROR, "Arquivo contém apenas cabeçalho: %s", filepath);
        fclose(file);
        mongodb_client_close(client);
        json_object_put(mapping);
        return NULL;
    }

    int count = 0;
    int file_lines = 0;
    int skipped_lines = 0;

    // Processa a primeira linha de dados que já foi lida
    file_lines++;
    // Remove quebra de linha do final
    line[strcspn(line, "\n")] = 0;

    // Cria um documento BSON simples
    bson_t *doc = bson_new();
    if (!doc) {
        logger_log(LOG_ERROR, "Erro ao criar documento BSON na linha %d", file_lines);
        skipped_lines++;
    } else {
        // Divide a linha em campos
        char **fields = NULL;
        int field_count = split_string(line, ";", &fields);
        if (field_count <= 0 || !fields) {
            logger_log(LOG_ERROR, "Erro ao dividir campos na linha %d", file_lines);
            skipped_lines++;
            bson_destroy(doc);
        } else {
            // Remove aspas de todos os campos
            for (int i = 0; i < field_count; i++) {
                if (fields[i]) {
                    char* clean_field = remove_quotes(fields[i]);
                    if (clean_field) {
                        free(fields[i]);
                        fields[i] = clean_field;
                    }
                }
            }

            // Adiciona campos básicos
            struct json_object* fields_obj;
            if (!json_object_object_get_ex(mapping, "fields", &fields_obj)) {
                logger_log(LOG_ERROR, "Erro ao obter campos do mapeamento");
                skipped_lines++;
                bson_destroy(doc);
                free_string_array(fields, field_count);
            } else {
                struct json_object_iterator it = json_object_iter_begin(fields_obj);
                struct json_object_iterator itEnd = json_object_iter_end(fields_obj);

                while (!json_object_iter_equal(&it, &itEnd)) {
                    const char* field_name = json_object_iter_peek_name(&it);
                    struct json_object* field_index_obj = json_object_iter_peek_value(&it);
                    int field_index = json_object_get_int(field_index_obj) - 1;  // Ajusta para índice 0-based

                    if (field_index >= 0 && field_index < field_count && fields && fields[field_index]) {
                        BSON_APPEND_UTF8(doc, field_name, fields[field_index]);
                    } else {
                        BSON_APPEND_UTF8(doc, field_name, "");
                    }

                    json_object_iter_next(&it);
                }

                // Cria o documento de contatos
                bson_t contatos;
                BSON_APPEND_DOCUMENT_BEGIN(doc, "contatos", &contatos);

                // Adiciona telefones
                struct json_object* contatos_obj;
                if (!json_object_object_get_ex(mapping, "contatos", &contatos_obj)) {
                    logger_log(LOG_ERROR, "Erro ao obter contatos do mapeamento");
                    skipped_lines++;
                    bson_destroy(doc);
                    free_string_array(fields, field_count);
                } else {
                    struct json_object* telefones_array_obj;
                    if (!json_object_object_get_ex(contatos_obj, "telefones", &telefones_array_obj)) {
                        logger_log(LOG_ERROR, "Erro ao obter telefones do mapeamento");
                        skipped_lines++;
                        bson_destroy(doc);
                        free_string_array(fields, field_count);
                    } else {
                        bson_t telefones;
                        BSON_APPEND_ARRAY_BEGIN(&contatos, "telefones", &telefones);
                        int telefone_index = 0;
                        int telefones_count = json_object_array_length(telefones_array_obj);

                        for (int i = 0; i < telefones_count; i++) {
                            struct json_object* telefone_index_obj = json_object_array_get_idx(telefones_array_obj, i);
                            if (!telefone_index_obj) continue;

                            int telefone_field_index = json_object_get_int(telefone_index_obj) - 1;  // Ajusta para índice 0-based
                            if (telefone_field_index >= 0 && telefone_field_index < field_count && 
                                fields[telefone_field_index] && 
                                strlen(fields[telefone_field_index]) > 0 && 
                                strcmp(fields[telefone_field_index], " ") != 0 && 
                                strcmp(fields[telefone_field_index], "-") != 0) {
                                char key[8];
                                bson_snprintf(key, sizeof(key), "%d", telefone_index++);
                                BSON_APPEND_UTF8(&telefones, key, fields[telefone_field_index]);
                            }
                        }
                        bson_append_array_end(&contatos, &telefones);

                        // Adiciona emails
                        struct json_object* emails_array_obj;
                        if (!json_object_object_get_ex(contatos_obj, "emails", &emails_array_obj)) {
                            logger_log(LOG_ERROR, "Erro ao obter emails do mapeamento");
                            skipped_lines++;
                            bson_destroy(doc);
                            free_string_array(fields, field_count);
                        } else {
                            bson_t emails;
                            BSON_APPEND_ARRAY_BEGIN(&contatos, "emails", &emails);
                            int email_index = 0;
                            int emails_count = json_object_array_length(emails_array_obj);

                            for (int i = 0; i < emails_count; i++) {
                                struct json_object* email_index_obj = json_object_array_get_idx(emails_array_obj, i);
                                if (!email_index_obj) continue;

                                int email_field_index = json_object_get_int(email_index_obj) - 1;  // Ajusta para índice 0-based
                                if (email_field_index >= 0 && email_field_index < field_count && 
                                    fields[email_field_index] && 
                                    strlen(fields[email_field_index]) > 0 && 
                                    strcmp(fields[email_field_index], " ") != 0 && 
                                    strcmp(fields[email_field_index], "-") != 0) {
                                    char key[8];
                                    bson_snprintf(key, sizeof(key), "%d", email_index++);
                                    BSON_APPEND_UTF8(&emails, key, fields[email_field_index]);
                                }
                            }
                            bson_append_array_end(&contatos, &emails);

                            bson_append_document_end(doc, &contatos);

                            if (!mongodb_client_insert(client, doc)) {
                                logger_log(LOG_ERROR, "Erro ao inserir documento na linha %d", file_lines);
                                skipped_lines++;
                            } else {
                                count++;
                                if (count % 1000 == 0) {
                                    logger_log(LOG_INFO, "Arquivo %s: %d registros importados", data->filename, count);
                                }
                            }
                        }
                    }
                }
            }
            free_string_array(fields, field_count);
        }
        bson_destroy(doc);
    }

    // Processa as demais linhas do arquivo
    while (getline(&line, &line_size, file) != -1) {
        file_lines++;
        // Remove quebra de linha do final
        line[strcspn(line, "\n")] = 0;

        // Cria um documento BSON simples
        bson_t *doc = bson_new();
        if (!doc) {
            logger_log(LOG_ERROR, "Erro ao criar documento BSON na linha %d", file_lines);
            skipped_lines++;
            continue;
        }

        // Divide a linha em campos
        char **fields = NULL;
        int field_count = split_string(line, ";", &fields);
        if (field_count <= 0 || !fields) {
            logger_log(LOG_ERROR, "Erro ao dividir campos na linha %d", file_lines);
            skipped_lines++;
            bson_destroy(doc);
            continue;
        }

        // Remove aspas de todos os campos
        for (int i = 0; i < field_count; i++) {
            if (fields[i]) {
                char* clean_field = remove_quotes(fields[i]);
                if (clean_field) {
                    free(fields[i]);
                    fields[i] = clean_field;
                }
            }
        }

        // Adiciona campos básicos
        struct json_object* fields_obj;
        if (!json_object_object_get_ex(mapping, "fields", &fields_obj)) {
            logger_log(LOG_ERROR, "Erro ao obter campos do mapeamento");
            skipped_lines++;
            bson_destroy(doc);
            free_string_array(fields, field_count);
            continue;
        }

        struct json_object_iterator it = json_object_iter_begin(fields_obj);
        struct json_object_iterator itEnd = json_object_iter_end(fields_obj);

        while (!json_object_iter_equal(&it, &itEnd)) {
            const char* field_name = json_object_iter_peek_name(&it);
            struct json_object* field_index_obj = json_object_iter_peek_value(&it);
            int field_index = json_object_get_int(field_index_obj) - 1;  // Ajusta para índice 0-based

            if (field_index >= 0 && field_index < field_count && fields && fields[field_index]) {
                BSON_APPEND_UTF8(doc, field_name, fields[field_index]);
            } else {
                BSON_APPEND_UTF8(doc, field_name, "");
            }

            json_object_iter_next(&it);
        }

        // Cria o documento de contatos
        bson_t contatos;
        BSON_APPEND_DOCUMENT_BEGIN(doc, "contatos", &contatos);

        // Adiciona telefones
        struct json_object* contatos_obj;
        if (!json_object_object_get_ex(mapping, "contatos", &contatos_obj)) {
            logger_log(LOG_ERROR, "Erro ao obter contatos do mapeamento");
            skipped_lines++;
            bson_destroy(doc);
            free_string_array(fields, field_count);
            continue;
        }

        struct json_object* telefones_array_obj;
        if (!json_object_object_get_ex(contatos_obj, "telefones", &telefones_array_obj)) {
            logger_log(LOG_ERROR, "Erro ao obter telefones do mapeamento");
            skipped_lines++;
            bson_destroy(doc);
            free_string_array(fields, field_count);
            continue;
        }

        bson_t telefones;
        BSON_APPEND_ARRAY_BEGIN(&contatos, "telefones", &telefones);
        int telefone_index = 0;
        int telefones_count = json_object_array_length(telefones_array_obj);

        for (int i = 0; i < telefones_count; i++) {
            struct json_object* telefone_index_obj = json_object_array_get_idx(telefones_array_obj, i);
            if (!telefone_index_obj) continue;

            int telefone_field_index = json_object_get_int(telefone_index_obj) - 1;  // Ajusta para índice 0-based
            if (telefone_field_index >= 0 && telefone_field_index < field_count && 
                fields[telefone_field_index] && 
                strlen(fields[telefone_field_index]) > 0 && 
                strcmp(fields[telefone_field_index], " ") != 0 && 
                strcmp(fields[telefone_field_index], "-") != 0) {
                char key[8];
                bson_snprintf(key, sizeof(key), "%d", telefone_index++);
                BSON_APPEND_UTF8(&telefones, key, fields[telefone_field_index]);
            }
        }
        bson_append_array_end(&contatos, &telefones);

        // Adiciona emails
        struct json_object* emails_array_obj;
        if (!json_object_object_get_ex(contatos_obj, "emails", &emails_array_obj)) {
            logger_log(LOG_ERROR, "Erro ao obter emails do mapeamento");
            skipped_lines++;
            bson_destroy(doc);
            free_string_array(fields, field_count);
            continue;
        }

        bson_t emails;
        BSON_APPEND_ARRAY_BEGIN(&contatos, "emails", &emails);
        int email_index = 0;
        int emails_count = json_object_array_length(emails_array_obj);

        for (int i = 0; i < emails_count; i++) {
            struct json_object* email_index_obj = json_object_array_get_idx(emails_array_obj, i);
            if (!email_index_obj) continue;

            int email_field_index = json_object_get_int(email_index_obj) - 1;  // Ajusta para índice 0-based
            if (email_field_index >= 0 && email_field_index < field_count && 
                fields[email_field_index] && 
                strlen(fields[email_field_index]) > 0 && 
                strcmp(fields[email_field_index], " ") != 0 && 
                strcmp(fields[email_field_index], "-") != 0) {
                char key[8];
                bson_snprintf(key, sizeof(key), "%d", email_index++);
                BSON_APPEND_UTF8(&emails, key, fields[email_field_index]);
            }
        }
        bson_append_array_end(&contatos, &emails);

        bson_append_document_end(doc, &contatos);

        if (!mongodb_client_insert(client, doc)) {
            logger_log(LOG_ERROR, "Erro ao inserir documento na linha %d", file_lines);
            skipped_lines++;
        } else {
            count++;
            if (count % 1000 == 0) {
                logger_log(LOG_INFO, "Arquivo %s: %d registros importados", data->filename, count);
            }
        }

        // Libera os campos
        free_string_array(fields, field_count);
        bson_destroy(doc);
    }

    free(line);
    fclose(file);

    // Atualiza os totais globais
    pthread_mutex_lock(&count_mutex);
    total_lines_read += file_lines;
    total_documents_inserted += count;
    pthread_mutex_unlock(&count_mutex);

    // Limpa
    mongodb_client_close(client);
    json_object_put(mapping);

    logger_log(LOG_INFO, "Arquivo %s concluído: %d registros importados", data->filename, count);
    return NULL;
}

int main() {
    // Inicializa o logger
    logger_init();
    logger_log(LOG_INFO, "Iniciando importação de arquivos CSV");

    // Registra o tempo de início
    time_t start_time = time(NULL);

    // Carrega as configurações
    Config *config = load_config("config/config.json");
    if (!config) {
        logger_log(LOG_ERROR, "Erro ao carregar configurações");
        return 1;
    }

    // Lista os arquivos do diretório
    DIR *dir = opendir("files_csv");
    if (!dir) {
        logger_log(LOG_ERROR, "Erro ao abrir diretório files_csv");
        return 1;
    }

    // Array para armazenar os nomes dos arquivos
    char **csv_files = NULL;
    int file_count = 0;
    struct dirent *entry;

    // Lê os arquivos do diretório
    while ((entry = readdir(dir)) != NULL) {
        if (is_valid_filename(entry->d_name)) {
            csv_files = realloc(csv_files, (file_count + 1) * sizeof(char*));
            csv_files[file_count] = strdup(entry->d_name);
            file_count++;
        }
    }
    closedir(dir);

    // Ordena os arquivos por nome
    qsort(csv_files, file_count, sizeof(char*), compare_files);

    // Ajusta o número máximo de threads para o número de arquivos
    config->max_threads = file_count;

    // Cria as threads
    pthread_t threads[MAX_THREADS];
    ThreadData thread_data[MAX_THREADS];

    for (int i = 0; i < file_count; i++) {
        thread_data[i].filename = csv_files[i];
        thread_data[i].config = config;
        pthread_create(&threads[i], NULL, process_file, &thread_data[i]);
    }

    // Aguarda as threads terminarem
    for (int i = 0; i < file_count; i++) {
        pthread_join(threads[i], NULL);
    }

    // Mostra estatísticas finais
    time_t end_time = time(NULL);
    double execution_time = difftime(end_time, start_time);
    
    printf("\nEstatísticas finais:\n");
    printf("Total de linhas lidas: %d\n", total_lines_read);
    printf("Total de documentos inseridos: %d\n", total_documents_inserted);
    printf("Tempo de execução: %.2f segundos\n", execution_time);

    // Limpa
    for (int i = 0; i < file_count; i++) {
        free(csv_files[i]);
    }
    free(csv_files);
    free_config(config);
    logger_log(LOG_INFO, "Importação concluída em %.2f segundos", execution_time);
    logger_close();

    return 0;
} 