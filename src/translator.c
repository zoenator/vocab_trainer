#include "translator.h"
#include "utils.h"

#include <ctype.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/typecheck-gcc.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
        char *memory;
        size_t size;
} MemoryStruct;

// Helper func to check API response
static int is_response_ok(const char *json)
{
    const char *status_ptr = strstr(json, "\"responseStatus\"");
    if (!status_ptr)
        return 0;

    // jump over key and skip ':' and space
    status_ptr += strlen("\"responseStatus\"");
    while (*status_ptr && (*status_ptr == ':' || isspace(*status_ptr)))
    {
        status_ptr++;
    }
    return (strncmp(status_ptr, "200", 3) == 0);
}

// helperfunc to get the translation from json
static int extract_translated_text(const char *json, char *dest, size_t dest_size)
{
    const char *needle = strstr(json, "\"translatedText\"");
    if (!needle)
        return -1;

    // find opening quotes of answer
    const char *start = strchr(needle + strlen("\"translatedText\""), ':');
    if (!start)
        return -1;

    start = strchr(start, '"');

    if (!start)
        return -1;

    start++; // jump to first char

    size_t i = 0;
    // iterate over each letter and escape if necessary
    while (*start && *start != '"' && i < dest_size - 1)
    {
        if (*start == '\\')
        {
            start++;
            if (*start == '"')
                dest[i++] = '"';
            else if (*start == '\\')
                dest[i++] = '\\';
            else if (*start == 'n')
                dest[i++] = '\n';
            else if (*start == 'u' && strncmp(start, "u0027", 5) == 0)
            {
                dest[i++] = '\'';
                start += 4;
            }
        }
        else
        {
            dest[i++] = *start;
        }
        start++;
    }
    dest[i] = '\0';
    return 0;
}

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = nmemb * size;
    MemoryStruct *mem = (MemoryStruct *)userp;

    char *tmp_ptr;
    tmp_ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (tmp_ptr == NULL)
    {
        PRINT_ERR("Allocation error");
        return 0;
    }

    mem->memory = tmp_ptr;
    memcpy(mem->memory + mem->size, contents, realsize);

    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int fetch_translation(const char *source_word, const char *source_lang, const char *target_lang, char *result_buffer, size_t buffer_size)
{
    if (source_word == NULL || target_lang == NULL || source_lang == NULL || buffer_size == 0 || result_buffer == NULL)
    {
        PRINT_USR_ERR("missing arguments");
        return 1;
    }

    MemoryStruct chunk;
    chunk.memory = malloc(1);
    if (chunk.memory == NULL)
    {
        PRINT_ERR("Allocation error");
        return -1;
    }
    // curl setup & call
    CURL *curl = curl_easy_init();
    if (curl == NULL)
    {
        PRINT_USR_ERR("Curl init failed");
        free(chunk.memory);
        return 1;
    }

    char url[256];
    char *escaped_word = curl_easy_escape(curl, source_word, 0);
    if (escaped_word == NULL)
    {
        PRINT_ERR("allcation error");
        free(chunk.memory);
        curl_easy_cleanup(curl);
        return -1;
    }
    chunk.size = 0;
    snprintf(url, sizeof(url), "https://api.mymemory.translated.net/get?q=%s&langpair=%s|%s", escaped_word, source_lang, target_lang);

    CURLcode result;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "VocabTrainer/1.0");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    result = curl_easy_perform(curl);
    if (result != CURLE_OK)
    {
        PRINT_USR_ERR("curl network transfer failed");
        free(chunk.memory);
        curl_free(escaped_word);
        curl_easy_cleanup(curl);
        return -1;
    }

    // check response status:
    char *response = strstr(chunk.memory, "responseStatus");
    if (is_response_ok(chunk.memory) != 1)
    {
        PRINT_USR_ERR("Response not okay");
        free(chunk.memory);
        curl_free(escaped_word);
        curl_easy_cleanup(curl);
        return -1;
    }

    if (extract_translated_text(chunk.memory, result_buffer, buffer_size) != 0)
    {
        PRINT_USR_ERR("Failed to parse transalted text");
        free(chunk.memory);
        curl_free(escaped_word);
        curl_easy_cleanup(curl);
        return -1;
    }

    curl_easy_cleanup(curl);
    curl_free(escaped_word);
    free(chunk.memory);
    return 0;
}