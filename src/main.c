#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define BUFFER_SIZE 2048
#define DEFAULT_CONFIG_FILE "simplqclient.config"

typedef struct {
    char *api_key;
    char *api_version;
    char *api_server;
    char *user_agent;
} SimplQClient;

typedef struct {
    char *data;
    size_t size;
} ResponseBuffer;

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total_size = size * nmemb;
    ResponseBuffer *buffer = (ResponseBuffer *)userp;

    char *ptr = realloc(buffer->data, buffer->size + total_size + 1);
    if (ptr == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        return 0;
    }

    buffer->data = ptr;
    memcpy(&(buffer->data[buffer->size]), contents, total_size);
    buffer->size += total_size;
    buffer->data[buffer->size] = 0;

    return total_size;
}

void build_url(const SimplQClient *client, const char *endpoint, char *url) {
    snprintf(url, BUFFER_SIZE, "https://%s/%s/%s", client->api_server, client->api_version, endpoint);
}

char *send_request(const SimplQClient *client, const char *method, const char *endpoint, const char *payload) {
    CURL *curl;
    CURLcode res;
    ResponseBuffer buffer = {0};

    char url[BUFFER_SIZE];
    build_url(client, endpoint, url);

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize cURL.\n");
        return NULL;
    }

    buffer.data = malloc(1);
    buffer.size = 0;

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    char api_key_header[BUFFER_SIZE];
    snprintf(api_key_header, BUFFER_SIZE, "x-api-key: %s", client->api_key);
    headers = curl_slist_append(headers, api_key_header);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&buffer);

    if (payload && (strcmp(method, "POST") == 0 || strcmp(method, "PATCH") == 0)) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    }

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "cURL error: %s\n", curl_easy_strerror(res));
        free(buffer.data);
        buffer.data = NULL;
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    return buffer.data;
}

void load_config(SimplQClient *client, const char *config_file) {
    FILE *file = fopen(config_file, "r");
    if (!file) {
        fprintf(stderr, "Failed to open config file: %s\n", config_file);
        exit(EXIT_FAILURE);
    }

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), file)) {
        char *key = strtok(line, "=");
        char *value = strtok(NULL, "\n");

        if (key && value) {
            if (strcmp(key, "API_KEY") == 0) {
                client->api_key = strdup(value);
            } else if (strcmp(key, "API_VERSION") == 0) {
                client->api_version = strdup(value);
            } else if (strcmp(key, "API_SERVER") == 0) {
                client->api_server = strdup(value);
            }
        }
    }

    fclose(file);
}

char* print_help(const char* message) {
    // Initial buffer size - adjust as needed
    char* buffer = malloc(2048 * sizeof(char));
    if (buffer == NULL) return NULL;  // Handle memory allocation failure
    
    int offset = 0;
    
    offset += snprintf(buffer + offset, 2048 - offset, "{\n");
    
    if (message != NULL) {
        offset += snprintf(buffer + offset, 2048 - offset, 
                          "  \"message\": \"%s\",\n", message);
    }
    
    offset += snprintf(buffer + offset, 2048 - offset, 
                      "  \"usage\": \"simplq-cli <command> [options]\",\n"
                      "  \"commands\": [\n"
                      "    {\"name\": \"create_queue\", \"args\": [\"json_payload\"]},\n"
                      "    {\"name\": \"get_queue\", \"args\": [\"queue_id\"]}\n"
                      "    {\"name\": \"update_queue\", \"args\": [\"queue_id\", \"json_attribute_values\"]},\n"
                      "    {\"name\": \"delete_queue\", \"args\": [\"queue_id\"]}\n"
                      "    {\"name\": \"get_all_queues\", \"args\": []},\n"
                      "    {\"name\": \"get_all_entries\", \"args\": [\"queue_id\"]},\n"
                      "    {\"name\": \"get_pending_entries\", \"args\": [\"queue_id\"]},\n"
                      "    {\"name\": \"get_processing_entries\", \"args\": [\"queue_id\"]},\n"
                      "    {\"name\": \"get_completed_entries\", \"args\": [\"queue_id\"]},\n"
                      "    {\"name\": \"get_error_entries\", \"args\": [\"queue_id\"]},\n"
                      "    {\"name\": \"get_next_entry\", \"args\": [\"queue_id\"]},\n"
                      "    {\"name\": \"get_entry\", \"args\": [\"queue_id\", \"entry_id\"]},\n"
                      "    {\"name\": \"create_entry\", \"args\": [\"queue_id\", \"payload\"]},\n"
                      "    {\"name\": \"update_entry\", \"args\": [\"queue_id\", \"entry_id\", \"json_attribute_values\"]},\n"
                      "    {\"name\": \"delete_entry\", \"args\": [\"queue_id\", \"entry_id\"]},\n"
                      "    {\"name\": \"set_entry_pending\", \"args\": [\"queue_id\", \"entry_id\", \"string_message\"]},\n"
                      "    {\"name\": \"set_entry_processing\", \"args\": [\"queue_id\", \"entry_id\", \"string_message\"]},\n"
                      "    {\"name\": \"set_entry_complete\", \"args\": [\"queue_id\", \"entry_id\", \"string_message\"]},\n"
                      "    {\"name\": \"set_entry_error\", \"args\": [\"queue_id\", \"entry_id\", \"string_message\"]},\n"
                      "  ],\n"
                      "  \"options\": [\n"
                      "    {\"flag\": \"--config\", \"description\": \"Specify a configuration file (default: simplqclient.config)\", \"value\": \"file\"}\n"
                      "  ]\n"
                      "}\n");

    return buffer;
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argv[1] == NULL) {
        print_help(NULL);
        return 1;
    }

    const char *config_file = DEFAULT_CONFIG_FILE;
    if (argc > 2 && strcmp(argv[1], "--config") == 0) {
        if (argc > 3) {
            config_file = argv[2];
            argv += 2;
            argc -= 2;
        } else {
            print_help(NULL);
            return 1;
        }
    }

    SimplQClient client = {0};
    client.user_agent = "C SimplQClient/1.0";

    load_config(&client, config_file);

    const char *command = argv[1];
    char *response = NULL;

    if (strcmp(command, "get_all_queues") == 0 || strcmp(command, "qs") == 0)
    	{
        response = send_request(&client, "GET", "queues", NULL);
		}

	else if (strcmp(command, "create_queue") == 0 || strcmp(command, "cq") == 0)
		{
        response = send_request(&client, "POST", "queues", NULL);
		}

	else if (strcmp(command, "get_queue") == 0 || strcmp(command, "q") == 0)
		{
		if (argc < 3) {
			printf("%s", "{ \"error\": \"Get queue requires a queue id\" }");
			return 1;
		}
        char endpoint[BUFFER_SIZE];
        snprintf(endpoint, BUFFER_SIZE, "queues/%s", argv[2]);
        response = send_request(&client, "GET", endpoint, NULL);
		}

	else if (strcmp(command, "delete_queue") == 0 || strcmp(command, "dq") == 0)
    	{
		if (argc < 3) {
			printf("%s", "{ \"error\": \"Delete queue requires a queue id\" }");
			return 1;
		}
        char endpoint[BUFFER_SIZE];
        snprintf(endpoint, BUFFER_SIZE, "queues/%s", argv[2]);
        response = send_request(&client, "DELETE", endpoint, NULL);
		}

    else if (strcmp(command, "get_all_entries") == 0 || strcmp(command, "es") == 0)
    	{
		if (argc < 3) {
			printf("%s", "{ \"error\": \"Get all entries requires a queue id\" }");
			return 1;
			}
        char endpoint[BUFFER_SIZE];
        snprintf(endpoint, BUFFER_SIZE, "queues/%s/entries", argv[2]);
        response = send_request(&client, "GET", endpoint, NULL);
		}

	else if (strcmp(command, "get_pending_entries") == 0)
		{
		if (argc < 3) {
			printf("%s", "{ \"error\": \"Get pending entries requires a queue id\" }");
			return 1;
			}
        char endpoint[BUFFER_SIZE];
        snprintf(endpoint, BUFFER_SIZE, "queues/%s/entries/pending", argv[2]);
        response = send_request(&client, "GET", endpoint, NULL);
		}

	else if (strcmp(command, "get_processing_entries") == 0)
		{
		if (argc < 3) {
			printf("%s", "{ \"error\": \"Get processing entries requires a queue id\" }");
			return 1;
			}
        char endpoint[BUFFER_SIZE];
        snprintf(endpoint, BUFFER_SIZE, "queues/%s/entries/processing", argv[2]);
        response = send_request(&client, "GET", endpoint, NULL);
		}

	else if (strcmp(command, "get_completed_entries") == 0)
		{
		if (argc < 3) {
			printf("%s", "{ \"error\": \"Get completed entries requires a queue id\" }");
			return 1;
			}
        char endpoint[BUFFER_SIZE];
        snprintf(endpoint, BUFFER_SIZE, "queues/%s/entries/completed", argv[2]);
        response = send_request(&client, "GET", endpoint, NULL);
		}

	else if (strcmp(command, "get_error_entries") == 0)
		{
		if (argc < 3) {
			printf("%s", "{ \"error\": \"Get completed entries requires a queue id\" }");
			return 1;
			}
        char endpoint[BUFFER_SIZE];
        snprintf(endpoint, BUFFER_SIZE, "queues/%s/entries/error", argv[2]);
        response = send_request(&client, "GET", endpoint, NULL);
        }

	else if (strcmp(command, "get_entry") == 0 || strcmp(command, "e") == 0)
		{
 		if (argc < 4) {
			printf("%s", "{ \"error\": \"Get entry requires a queue id and an entry id\" }");
			return 1;
			}
       char endpoint[BUFFER_SIZE];
        snprintf(endpoint, BUFFER_SIZE, "queues/%s/entries/%s", argv[2], argv[3]);
        response = send_request(&client, "GET", endpoint, NULL);
		}

	else if (strcmp(command, "get_next_entry") == 0)
		{
		if (argc < 3) {
			printf("%s", "{ \"error\": \"Get next entry requires a queue id\" }");
			return 1;
			}
        char endpoint[BUFFER_SIZE];
        snprintf(endpoint, BUFFER_SIZE, "queues/%s/entries/next", argv[2]);
        response = send_request(&client, "GET", endpoint, NULL);
		}

	else if (strcmp(command, "update_entry") == 0)
		{
 		if (argc < 5) {
			printf("%s", "{ \"error\": \"Update entry requires a queue id, an entry id, and a JSON payload\" }");
			return 1;
			}
        char endpoint[BUFFER_SIZE];
        snprintf(endpoint, BUFFER_SIZE, "queues/%s/entries/%s", argv[2], argv[3]);
        response = send_request(&client, "PATCH", endpoint, argv[4]);
		}

	else if (strcmp(command, "create_entry") == 0 || strcmp(command, "ce") == 0)
		{
		if (argc < 3) {
			printf("%s", "{ \"error\": \"Create entry requires a queue id\" }");
			return 1;
			}
        char endpoint[BUFFER_SIZE];
        snprintf(endpoint, BUFFER_SIZE, "queues/%s/entries", argv[2]);
        response = send_request(&client, "POST", endpoint, argv[3]);
		}

	else if (strcmp(command, "delete_entry") == 0 || strcmp(command, "de") == 0)
		{
 		if (argc < 4) {
			printf("%s", "{ \"error\": \"Delete entry requires a queue id and an entry id\" }");
			return 1;
			}
        char endpoint[BUFFER_SIZE];
        snprintf(endpoint, BUFFER_SIZE, "queues/%s/entries/%s", argv[2], argv[3]);
        response = send_request(&client, "DELETE", endpoint, NULL);
		}

	else if (strcmp(command, "set_entry_pending") == 0)
		{
 		if (argc < 4) {
			printf("%s", "{ \"error\": \"Set entry pending requires a queue id and an entry id\" }");
			return 1;
			}
        char endpoint[BUFFER_SIZE];
        snprintf(endpoint, BUFFER_SIZE, "queues/%s/entries/%s/pending", argv[2], argv[3]);
        response = send_request(&client, "PATCH", endpoint, argv[4]);
        }

	else if (strcmp(command, "set_entry_processing") == 0)
		{
 		if (argc < 4) {
			printf("%s", "{ \"error\": \"Set entry processing requires a queue id and an entry id\" }");
			return 1;
			}
        char endpoint[BUFFER_SIZE];
        snprintf(endpoint, BUFFER_SIZE, "queues/%s/entries/%s/processing", argv[2], argv[3]);
        response = send_request(&client, "PATCH", endpoint, argv[4]);
		}

	else if (strcmp(command, "set_entry_complete") == 0)
		{
 		if (argc < 4) {
			printf("%s", "{ \"error\": \"Set entry complete requires a queue id and an entry id\" }");
			return 1;
			}
        char endpoint[BUFFER_SIZE];
        snprintf(endpoint, BUFFER_SIZE, "queues/%s/entries/%s/complete", argv[2], argv[3]);
        response = send_request(&client, "PATCH", endpoint, argv[4]);
        }

	else if (strcmp(command, "set_entry_error") == 0)
		{
 		if (argc < 4) {
			printf("%s", "{ \"error\": \"Set entry error requires a queue id and an entry id\" }");
			return 1;
			}
        char endpoint[BUFFER_SIZE];
        snprintf(endpoint, BUFFER_SIZE, "queues/%s/entries/%s/error", argv[2], argv[3]);
        response = send_request(&client, "PATCH", endpoint, argv[4]);
		}

	else if (strcmp(command, "update_queue") == 0 || strcmp(command, "uq") == 0) {
 		if (argc < 4) {
			printf("%s", "{ \"error\": \"Update queue requires a queue id and a payload\" }");
			return 1;
			}
        char endpoint[BUFFER_SIZE];
        snprintf(endpoint, BUFFER_SIZE, "queues/%s", argv[2]);
        response = send_request(&client, "PATCH", endpoint, argv[3]);
		}

	else {
		char* help_text = print_help(NULL);
		if (help_text != NULL) {
			printf("%s", help_text);
			free(help_text);
		}
		return 1;
    }

    if (response) {
        printf("%s\n", response);
        free(response);
    }

    free(client.api_key);
    free(client.api_version);
    free(client.api_server);

    return 0;
}
