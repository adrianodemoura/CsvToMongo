#include "mongodb_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int total_documents = 0;

MongoDBClient* mongodb_client_init(const char *uri, const char *database, const char *collection) {
    MongoDBClient *client = (MongoDBClient*)malloc(sizeof(MongoDBClient));
    if (!client) return NULL;

    // Inicializa o driver MongoDB
    mongoc_init();

    // Cria o cliente
    client->client = mongoc_client_new(uri);
    if (!client->client) {
        fprintf(stderr, "Erro ao criar cliente MongoDB\n");
        free(client);
        return NULL;
    }

    client->database = mongoc_client_get_database(client->client, database);
    if (!client->database) {
        mongoc_client_destroy(client->client);
        free(client);
        return NULL;
    }

    // Obtém a coleção
    client->collection = mongoc_client_get_collection(client->client, database, collection);
    if (!client->collection) {
        fprintf(stderr, "Erro ao obter coleção MongoDB\n");
        mongoc_database_destroy(client->database);
        mongoc_client_destroy(client->client);
        free(client);
        return NULL;
    }

    return client;
}

void mongodb_client_close(MongoDBClient *client) {
    if (client) {
        if (client->collection) mongoc_collection_destroy(client->collection);
        if (client->database) mongoc_database_destroy(client->database);
        if (client->client) mongoc_client_destroy(client->client);
        free(client);
    }
}

bool mongodb_client_insert(MongoDBClient *client, bson_t *doc) {
    if (!client || !client->collection || !doc) return false;

    bson_error_t error;
    bool result = mongoc_collection_insert_one(client->collection, doc, NULL, NULL, &error);
    if (!result) {
        fprintf(stderr, "Erro ao inserir documento: %s\n", error.message);
    } else {
        total_documents++;
    }
    return result;
}

bool mongodb_client_insert_to_collection(MongoDBClient *client, const char *collection_name, bson_t *doc) {
    if (!client || !client->client || !collection_name || !doc) return false;

    mongoc_collection_t *collection = mongoc_client_get_collection(client->client, 
        mongoc_database_get_name(client->database), collection_name);
    if (!collection) return false;

    bson_error_t error;
    bool result = mongoc_collection_insert_one(collection, doc, NULL, NULL, &error);
    if (!result) {
        fprintf(stderr, "Erro ao inserir documento na coleção %s: %s\n", collection_name, error.message);
    }

    mongoc_collection_destroy(collection);
    return result;
}

void mongodb_client_print_stats() {
    printf("\nTotal de documentos inseridos: %d\n", total_documents);
} 