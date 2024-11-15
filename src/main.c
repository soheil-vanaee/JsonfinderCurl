#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>

#define BUFFER_SIZE 2048

struct MemoryStruct {
    char *memory;
    size_t size;
};

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t totalSize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + totalSize + 1);
    if (ptr == NULL) {
        printf("Error reallocating memory\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, totalSize);
    mem->size += totalSize;
    mem->memory[mem->size] = '\0';

    return totalSize;
}

char *fetch_json(const char *url) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            free(chunk.memory);
            chunk.memory = NULL;
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return chunk.memory;
}

void search_json(json_t *root, const char *key) {
    const char *json_key;
    json_t *value;

    json_object_foreach(root, json_key, value) {
        if (strcmp(json_key, key) == 0) {
            if (json_is_string(value)) {
                printf("%s: %s\n", json_key, json_string_value(value));
            } else if (json_is_number(value)) {
                printf("%s: %f\n", json_key, json_number_value(value));
            } else {
                printf("%s: (type not supported)\n", json_key);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3 || strcmp(argv[1], "-search") != 0) {
        fprintf(stderr, "Usage: %s -search <key>\n", argv[0]);
        return 1;
    }

    const char *search_key = argv[2];
    const char *url = "https://examples.com"; 

    char *json_data = fetch_json(url);
    if (json_data == NULL) {
        fprintf(stderr, "Failed to fetch JSON data\n");
        return 1;
    }

    json_error_t error;
    json_t *root = json_loads(json_data, 0, &error);
    free(json_data);

    if (!root) {
        fprintf(stderr, "Error parsing JSON: %s\n", error.text);
        return 1;
    }

    search_json(root, search_key);

    json_decref(root);
    return 0;
}
