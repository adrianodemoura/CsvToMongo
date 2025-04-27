#ifndef STRING_UTILS_H
#define STRING_UTILS_H

// Divide uma string em um array de strings usando o delimitador especificado
// Retorna o número de campos encontrados
// O array resultante deve ser liberado com free_string_array
int split_string(const char *str, const char *delim, char ***result);

// Libera um array de strings alocado por split_string
void free_string_array(char **array, int count);

#endif // STRING_UTILS_H 