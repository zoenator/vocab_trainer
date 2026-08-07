#include "paths.h"

#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// static variables to safe paths after first initialization
static char *config_filepath = NULL;
static char *storage_filepath = NULL;
static int paths_initialized = 0; // flag to check if paths have been initialized

// free func for atexit
void free_filepaths()
{
    if (config_filepath)
        free(config_filepath);
    if (storage_filepath)
        free(storage_filepath);
}

// Creates needed directories for a given filepath if they do not already exist
static int create_needed_directories(const char *filepath)
{
    // char *dirpath;
    char *dirpath = calloc(strlen(filepath) + 1, sizeof(char));
    if (dirpath == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    snprintf(dirpath, strlen(filepath) + 1, "%s", filepath);

    // Go through path and create each directory if it doesn't exist
    for (char *p = dirpath + 1; *p; p++)
    {
        if (*p == '/') // temporary replace '/' with '\0' to isolate directory path
        {
            *p = '\0';
            if (mkdir(dirpath, 0755) != 0 && errno != EEXIST) // create directory if it doesn't exist
            {
                fprintf(stderr, "Error: Could not create directory path for %s\n", filepath); // error
                free(dirpath);
                return -1;
            }
            *p = '/'; // restore '/'
        }
    }
    if (mkdir(dirpath, 0755) != 0 && errno != EEXIST)
    {
        fprintf(stderr, "Error: Could not create directory path for %s\n", filepath);
        free(dirpath);
        return -1;
    }
    free(dirpath);
    return 0;
}

// Initializes config and storage file paths based on HOME env variable if not already done
static void initialize_paths()
{
    const char dotconfig[] = "/.config/vocab_trainer";
    const char dotlocal[] = "/.local/share/vocab_trainer";
    if (paths_initialized)
        return; // already initialized

    atexit(free_filepaths);

    const char *home_dir = getenv("HOME");
    if (home_dir == NULL)
    {
        fprintf(stderr, "Error: HOME environment variable not set.\n");
        exit(EXIT_FAILURE);
    }

    // Config path
    size_t config_dir_len = strlen(home_dir) + strlen(dotconfig) + 1;
    char *config_dir = calloc(config_dir_len, sizeof(char));
    if (config_dir == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    snprintf(config_dir, config_dir_len, "%s%s", home_dir, dotconfig);
    if (create_needed_directories(config_dir) == 0)
    {
        const char *filename = "/vocab.cfg";
        size_t len = strlen(config_dir) + strlen(filename) + 1;
        if (len > PATH_MAX)
        {
            fprintf(stderr, "Error: Config file path (%zu) exceeds PATH_MAX (%d).\n", len, PATH_MAX);
            exit(EXIT_FAILURE);
        }

        config_filepath = calloc(len, sizeof(char));
        if (config_filepath == NULL)
        {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            exit(EXIT_FAILURE);
        }
        snprintf(config_filepath, len, "%s%s", config_dir, filename);
    }

    // Storage path
    size_t storage_dir_len = strlen(home_dir) + strlen(dotlocal) + 1;
    char *storage_dir = calloc(storage_dir_len, sizeof(char));
    if (storage_dir == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    snprintf(storage_dir, storage_dir_len, "%s%s", home_dir, dotlocal);
    if (create_needed_directories(storage_dir) == 0)
    {
        const char *filename = "/vocab.bin";
        size_t len = strlen(storage_dir) + strlen(filename) + 1;

        if (len > PATH_MAX)
        {
            fprintf(stderr, "Error: Storage file path (%zu) exceeds PATH_MAX (%d).\n", len, PATH_MAX);
            exit(EXIT_FAILURE);
        }

        storage_filepath = calloc(len, sizeof(char));
        if (storage_filepath == NULL)
        {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            exit(EXIT_FAILURE);
        }
        snprintf(storage_filepath, len, "%s%s", storage_dir, filename);
    }
    free(storage_dir);
    free(config_dir);

    paths_initialized = 1;
}

const char *get_storage_filepath()
{
    initialize_paths();
    return storage_filepath;
}

const char *get_config_filepath()
{
    initialize_paths();
    return config_filepath;
}
