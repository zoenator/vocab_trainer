#include "paths.h"
#include "vocab_entry.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int cmd_add(int argc, char **argv)
{
    // TODO This needs a more dynamic redesign (Here you cant type in gender without giving a tpye too which is kinda meh)
    if (argc < 5)
    {
        printf("Usage: Vocab_CLI add <lang> <front_text> <back_text> [entry_type] [tags]");
        return 1;
    }

    char *entry_type = (argc >= 6) ? argv[5] : "0";
    const char *tags = (argc >= 7) ? argv[6] : "0";

    char msg[512];
    snprintf(msg, sizeof(msg), "ADD|%s|%s|%s|%s|%s", argv[2], argv[3], argv[4], entry_type, tags);
    int fd = open(PIPE_PATH, O_WRONLY);
    if (fd == -1)
    {
        printf("Error: Pipe not found.\n");
        return 1;
    }
    write(fd, msg, strlen(msg));
    close(fd);
    return 0;
}

int cmd_stats(int argc, char **argv)
{
    FILE *f = fopen(get_storage_filepath(), "rb");
    if (f == NULL)
    {
        printf("Error: No vocabs yet.\n");
        return 1;
    }

    // * read binary and calculate stats
    vocab_entry ve;
    int exit_status = 0;
    // stat variables
    // TODO expand stats (e.G solved exercises/day ... )
    int total_words = 0;
    int *different_languages = calloc(2, sizeof(int));
    char **languages = calloc(2, sizeof(char *));
    int capacity = 2;
    if (languages == NULL)
    {
        printf("alloc error..\n");
        return 1;
    }
    while (fread(&ve, sizeof(vocab_entry), 1, f))
    {
        total_words++;

        unsigned int exists = 0;
        unsigned int count = 0;
        for (int i = 0; languages[i] != NULL; i++)
        {
            if (strcmp(ve.language, languages[i]) == 0)
            {
                exists = 1;
                break;
            }
            count++;
        }
        if (!exists)
        {
            if (count >= capacity)
            {
                void *tmp = realloc(different_languages, sizeof(int) * capacity * 2);
                if (tmp == NULL)
                {
                    printf("realloc error\n");
                    exit_status = 1;
                    goto EXIT;
                }
                different_languages = tmp;
                memset(different_languages + capacity, 0, sizeof(int) * capacity);
                tmp = realloc(languages, sizeof(char *) * capacity * 2);
                if (tmp == NULL)
                {
                    printf("realloc error\n");
                    exit_status = 1;
                    goto EXIT;
                }
                languages = tmp;
                memset(languages + capacity, 0, sizeof(char *) * capacity);
                capacity *= 2;
            }

            languages[count] = strdup(ve.language);
            different_languages[count]++;
        }
        else
        {
            different_languages[count]++;
        }
    }

    fclose(f);

    printf("Total words: %i\n", total_words);

EXIT:
    for (int i = 0; languages[i] != NULL; i++)
    {
        printf("%s words: %d\n", languages[i], different_languages[i]);
        free(languages[i]);
    }

    free(languages);
    free(different_languages);

    return exit_status;
}

int cmd_mode(int argc, char **argv)
{
    if (argc < 2 || strcmp(argv[2], "PASSIVE") != 0 || strcmp(argv[2], "ACTIVE") != 0)
    {
        printf("Usage: VocabTrainer mode <mode> \n Modes: \n'PASSIVE'\n'ACTIVE'\n");
        return 1;
    }

    char msg[128];
    snprintf(msg, sizeof(msg), "MODE|%s", argv[2]);
    int fd = open(PIPE_PATH, O_WRONLY);
    if (fd == -1)
    {
        printf("Error: Pipe not found.\n");
        return 1;
    }
    write(fd, msg, strlen(msg));
    close(fd);
    return 0;
}

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        printf("Usage: Vocab_cli <command> [args]\nCommands: add, stats\n");
        return 1;
    }

    if (strcmp(argv[1], "add") == 0)
    {
        return cmd_add(argc, argv);
    }
    else if (strcmp(argv[1], "stats") == 0)
    {
        return cmd_stats(argc, argv);
    }
    else if (strcmp(argv[1], "mode") == 0)
    {
        return cmd_mode(argc, argv);
    }
    else
    {
        printf("Unknown command: %s\n", argv[1]);
        return 1;
    }

    return 0;
}