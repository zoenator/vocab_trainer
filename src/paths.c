#include "paths.h"
#include "utils.h"

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
        PRINT_ERR("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    snprintf(dirpath, strlen(filepath) + 1, "%s", filepath);

    // Go through path and create each directory if it doesn't exist
    for (char *p = dirpath + 1; *p; p++)
    {
        if (*p != '/') // temporary replace '/' with '\0' to isolate directory path
        {
            continue;
        }

        *p = '\0';
        if (mkdir(dirpath, 0755) != 0 && errno != EEXIST) // create directory if it doesn't exist
        {
            PRINT_USR_ERR("Error: Could not create directory path for %s", filepath); // error
            free(dirpath);
            return -1;
        }

        *p = '/'; // restore '/'
    }
    if (mkdir(dirpath, 0755) != 0 && errno != EEXIST)
    {
        PRINT_USR_ERR("Error: Could not create directory path for %s", filepath);
        free(dirpath);
        return -1;
    }
    free(dirpath);
    return 0;
}

// Initializes config and storage file paths based on HOME env variable if not already done
static int initialize_paths()
{
    const char dotconfig[] = "/.config/vocab_trainer";
    const char dotlocal[] = "/.local/share/vocab_trainer";

    char *config_dir = NULL;
    char *storage_dir = NULL;
    int error = 0;

    if (paths_initialized)
        return 0; // already initialized

    const char *home_dir = getenv("HOME");
    if (home_dir == NULL)
    {
        PRINT_USR_ERR("Error: HOME environment variable not set");
        error = 1;
        goto cleanup;
    }

    // Config path
    size_t config_dir_len = strlen(home_dir) + strlen(dotconfig) + 1;
    config_dir = calloc(config_dir_len, sizeof(char));
    if (config_dir == NULL)
    {
        PRINT_ERR("Memory allocation failed");
        error = 1;
        goto cleanup;
    }
    snprintf(config_dir, config_dir_len, "%s%s", home_dir, dotconfig);
    if (create_needed_directories(config_dir) != 0)
    {
        error = 1;
        goto cleanup;
    }

    const char *filename = "/vocab.cfg";
    size_t len = strlen(config_dir) + strlen(filename) + 1;
    if (len > PATH_MAX)
    {
        PRINT_USR_ERR("Error: Storage file path (%zu) exceeds PATH_MAX (%d).", len, PATH_MAX);
        error = 1;
        goto cleanup;
    }

    config_filepath = calloc(len, sizeof(char));
    if (config_filepath == NULL)
    {
        PRINT_ERR("Memory allocation failed");
        error = 1;
        goto cleanup;
    }
    snprintf(config_filepath, len, "%s%s", config_dir, filename);

    // Storage path
    size_t storage_dir_len = strlen(home_dir) + strlen(dotlocal) + 1;
    storage_dir = calloc(storage_dir_len, sizeof(char));
    if (storage_dir == NULL)
    {
        PRINT_ERR("Memory allocation failed");
        error = 1;
        goto cleanup;
    }
    snprintf(storage_dir, storage_dir_len, "%s%s", home_dir, dotlocal);
    if (create_needed_directories(storage_dir) != 0)
    {
        error = 1;
        goto cleanup;
    }

    const char *filename_storage = "/vocab.bin";
    len = strlen(storage_dir) + strlen(filename) + 1;

    if (len > PATH_MAX)
    {
        PRINT_USR_ERR("Error: Storage file path (%zu) exceeds PATH_MAX (%d).", len, PATH_MAX);
        error = 1;
        goto cleanup;
    }

    storage_filepath = calloc(len, sizeof(char));
    if (storage_filepath == NULL)
    {
        PRINT_ERR("Memory allocation failed");
        error = 1;
        goto cleanup;
    }
    snprintf(storage_filepath, len, "%s%s", storage_dir, filename);

    paths_initialized = 1;
    atexit(free_filepaths);
cleanup:
    free(storage_dir);
    free(config_dir);
    if (error)
        free_filepaths();
    return error;
}

const char *get_storage_filepath()
{
    if (initialize_paths() == 1)
        return NULL;
    return storage_filepath;
}

const char *get_config_filepath()
{
    if (initialize_paths() == 1)
        return NULL;
    return config_filepath;
}
