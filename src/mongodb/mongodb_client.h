#ifndef MONGODB_CLIENT_H
#define MONGODB_CLIENT_H

#include <bson/bson.h>
#include <mongoc/mongoc.h>

typedef struct {
    mongoc_client_t *client;
    mongoc_database_t *database;
    mongoc_collection_t *collection;
} MongoDBClient;

// Inicializa o cliente MongoDB
MongoDBClient* mongodb_client_init(const char *uri, const char *database, const char *collection);

// Fecha a conexão com o MongoDB
void mongodb_client_close(MongoDBClient *client);

// Insere um documento no MongoDB
bool mongodb_client_insert(MongoDBClient *client, bson_t *doc);

bool mongodb_client_insert_to_collection(MongoDBClient *client, const char *collection_name, bson_t *doc);

void mongodb_client_print_stats();

#endif // MONGODB_CLIENT_H 