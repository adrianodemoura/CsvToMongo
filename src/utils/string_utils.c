#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_utils.h"

int split_string(const char *str, const char *delim, char ***result) {
    if (!str || !delim || !result) {
        fprintf(stderr, "Erro: parâmetros inválidos em split_string\n");
        return 0;
    }

    // Conta o número de campos
    int count = 1;  // Começa com 1 para contar o último campo
    const char *p = str;
    while ((p = strstr(p, delim)) != NULL) {
        count++;
        p += strlen(delim);
    }

    // Aloca o array de resultados
    *result = calloc(count, sizeof(char*));
    if (!*result) {
        fprintf(stderr, "Erro: falha ao alocar memória para array de resultados\n");
        return 0;
    }

    // Preenche o array com os campos
    const char *start = str;
    const char *end;
    int i = 0;

    while ((end = strstr(start, delim)) != NULL) {
        size_t len = end - start;
        (*result)[i] = malloc(len + 1);
        if (!(*result)[i]) {
            fprintf(stderr, "Erro: falha ao alocar memória para campo %d\n", i);
            // Em caso de erro, libera o que já foi alocado
            for (int j = 0; j < i; j++) {
                free((*result)[j]);
            }
            free(*result);
            *result = NULL;
            return 0;
        }
        strncpy((*result)[i], start, len);
        (*result)[i][len] = '\0';
        i++;
        start = end + strlen(delim);
    }

    // Adiciona o último campo
    size_t len = strlen(start);
    (*result)[i] = malloc(len + 1);
    if (!(*result)[i]) {
        fprintf(stderr, "Erro: falha ao alocar memória para o último campo\n");
        // Em caso de erro, libera o que já foi alocado
        for (int j = 0; j < i; j++) {
            free((*result)[j]);
        }
        free(*result);
        *result = NULL;
        return 0;
    }
    strcpy((*result)[i], start);

    return count;
}

void free_string_array(char **array, int count) {
    if (!array) {
        return;
    }

    for (int i = 0; i < count; i++) {
        if (array[i]) {
            free(array[i]);
        }
    }
    free(array);
} 