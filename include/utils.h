#pragma once
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define PRINT_ERR(msg)                                                                                                                     \
    do                                                                                                                                     \
    {                                                                                                                                      \
        fprintf(stderr, "[%s:%d] in %s(): %s: %s\n", __FILE__, __LINE__, __func__, msg, strerror(errno));                                  \
    } while (0)

#define PRINT_USR_ERR(msg, ...)                                                                                                            \
    do                                                                                                                                     \
    {                                                                                                                                      \
        fprintf(stderr, "[%s:%d] in %s():" msg "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__);                                         \
    } while (0)

int parse_lueckentext(const char *input, char *display_text, char *solution, size_t display_size, size_t solution_size);